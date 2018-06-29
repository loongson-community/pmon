/* whd: loongson3C_ddr3_leveling.S
   2012.9.1
   first written by Leping from pfunc.s
   USE t8 to pass the CONFIG address
   ECC slice in not included yet
   2012.9.25 add ECC slice
*/
#include "ddr_dir/ddr_leveling_define.h"
#define	PREMBLE_CHECK_DEBUG
//#define	PRINT_PREMBLE_CHECK
#define PRINT_DDR_LEVELING
//#define	FILTER_CHECK
//#define SIGNAL_DEPICT_DEBUG
//#define	FIND_PREMBLE_LENGTH
//#define LVL_DEBUG
/* in PRINTSTR: a0, a1, a2, v0, v1 will be changed */
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
    li      t0, 0x8
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1

1:

    dli     t1, 0x28
    or      t1, t1, t8

2:
    ld      a0, 0x0(t1)
    dli     t4, 0x000000000000
//    dli     t4, 0x020200000000
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

#if 1 //3a3000
write_leveling:
    PRINTSTR("\r\nwrite leveling begin\r\n")

/* 2. set all dll to be 0 */
/* identify wheather there is ecc slice */
    li      t0, 0x8
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
    dli     t1, 0x018
    or      t1, t1, t8
dll_wrdqs_set0:
    daddu   t1, t1, 0x20
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffff00ffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    subu    t0, t0, 0x1 
    bnez    t0, dll_wrdqs_set0 
    nop
 
    PRINTSTR("\r\nall dll_wrdqs set 0\r\n")

/* 3. set leveling mode to be WRITE LEVELING */
lvl_mode_set01:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    ori     a0, a0, 0x1
    sd      a0, 0x0(t1)

    PRINTSTR("\r\nset leveling mode to be WRITE LEVELING\r\n")

/* 4. check whether to start leveling */
lvl_ready_sampling:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000ff0000000000
    and     a0, a0, t4
    beqz    a0, lvl_ready_sampling
    nop

    PRINTSTR("\r\nwrite leveling ready\r\n")

/* 5. Set leveling req */

/* t3 is used to indicate whether all slice got 0 */
	dli		t0, 0x0 //t0 is used to indicate the number of slice being filtering check
    dli     t3, 0x0
    dli     a1, 0x0
    dli     s6, 0x0
	dli		a3, 0x40
lvl_req_set:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffff00ff
    and     a0, a0, t4
    ori     a0, a0, 0x100
    sd      a0, 0x0(t1)
    and     a0, a0, t4
    sd      a0, 0x0(t1)

#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrite leveling req\r\n")
#endif

/* 6. check whether this leveling request done */
lvl_done_sampling:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00ff000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 48
    beqz    a0, lvl_done_sampling
    nop

#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrite leveling done\r\n")
#endif

    bnez    t3, lvl_resp_set
    nop 

/* 7. check each slice response to adjust the dll */

/* 7.1 ensure all slice got a 0 first */
/* a2 is used to indicate whether any slice got an 1 */
//    dli     s6, 0x1
lvl_resp_0_set0:
    dli     s7, 0x0
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0100000000000000
    and     a0, a0, t4
    //dsrl    a0, a0, 56
    //beq     a0, a1, dll_wrdqs_0_add1
    bnez    a0, dll_wrdqs_0_add1
    nop
lvl_resp_1_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000000001
    and     a0, a0, t4
    //beq     a0, a1, dll_wrdqs_1_add1
    bnez    a0, dll_wrdqs_1_add1
    nop
lvl_resp_2_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000000100
    and     a0, a0, t4
    //dsrl    a0, a0, 8
    //beq     a0, a1, dll_wrdqs_2_add1
    bnez    a0, dll_wrdqs_2_add1
    nop
lvl_resp_3_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000010000
    and     a0, a0, t4
    //dsrl    a0, a0, 16
    //beq     a0, a1, dll_wrdqs_3_add1
    bnez    a0, dll_wrdqs_3_add1
    nop
lvl_resp_4_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000001000000
    and     a0, a0, t4
    //dsrl    a0, a0, 24
    //beq     a0, a1, dll_wrdqs_4_add1
    bnez    a0, dll_wrdqs_4_add1
    nop
lvl_resp_5_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000100000000
    and     a0, a0, t4
    //dsrl    a0, a0, 32
    //beq     a0, a1, dll_wrdqs_5_add1
    bnez    a0, dll_wrdqs_5_add1
    nop
lvl_resp_6_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000010000000000
    and     a0, a0, t4
    //dsrl    a0, a0, 40
    //beq     a0, a1, dll_wrdqs_6_add1
    bnez    a0, dll_wrdqs_6_add1
    nop
lvl_resp_7_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0001000000000000
    and     a0, a0, t4
    //dsrl    a0, a0, 48
    //beq     a0, a1, dll_wrdqs_7_add1
    bnez    a0, dll_wrdqs_7_add1
    nop

lvl_resp_8_set0:
/* identify wheather there is ecc slice */
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop

    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0100000000000000
    and     a0, a0, t4
    //dsrl    a0, a0, 56
    //beq     a0, a1, dll_wrdqs_8_add1
    bnez    a0, dll_wrdqs_8_add1
    nop

1:
    bnez    s7, lvl_req_set
    nop

#if 1
/* filter the 0 to 1 glitch, which will cause the reboot error*/
additional_lvl_req:
    blt     s6, 5, dll_wrdqs0_add
    nop
    blt     s6, 10, dll_wrdqs1_add
    nop
    blt     s6, 15, dll_wrdqs2_add
    nop
    blt     s6, 20, dll_wrdqs3_add
    nop
    blt     s6, 25, dll_wrdqs4_add
    nop
    blt     s6, 30, dll_wrdqs5_add
    nop
    blt     s6, 35, dll_wrdqs6_add
    nop
    blt     s6, 40, dll_wrdqs7_add
    nop
    //ECC
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 11f
    nop
    blt     s6, 45, dll_wrdqs8_add
    nop
11:
    b       2221f
    nop

dll_wrdqs0_add:
    daddu   s6, s6, 0x1
    ld      a0, 0x38(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x38(t8)

#if 1
	lb		a0, 0x3a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	or		a0, a0, t4
	sb		a0, 0x39(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x21(t8)
	lb		a0, 0x3a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x21(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x20(t8)
	lb		a0, 0x39(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x20(t8)
1:
#endif	
    b       lvl_req_set //make additional 5 times lvl_req
    nop

dll_wrdqs1_add:
    daddu   s6, s6, 0x1
    ld      a0, 0x58(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x58(t8)
	
	lb		a0, 0x5a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	or		a0, a0, t4
	sb		a0, 0x59(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x41(t8)
	lb		a0, 0x5a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x41(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x40(t8)
	lb		a0, 0x59(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x40(t8)
1:
    
	b       lvl_req_set //make additional 5 times lvl_req
    nop

dll_wrdqs2_add:
    daddu   s6, s6, 0x1
    ld      a0, 0x78(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x78(t8)
	
	lb		a0, 0x7a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	or		a0, a0, t4
	sb		a0, 0x79(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x61(t8)
	lb		a0, 0x7a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x61(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x60(t8)
	lb		a0, 0x79(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x60(t8)
1:
    
    b       lvl_req_set //make additional 5 times lvl_req
    nop

dll_wrdqs3_add:
    daddu   s6, s6, 0x1
    ld      a0, 0x98(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x98(t8)
	
	lb		a0, 0x9a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	or		a0, a0, t4
	sb		a0, 0x99(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x81(t8)
	lb		a0, 0x9a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x81(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x80(t8)
	lb		a0, 0x99(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x80(t8)
1:
    
    b       lvl_req_set //make additional 5 times lvl_req
    nop

dll_wrdqs4_add:
    daddu   s6, s6, 0x1
    ld      a0, 0xb8(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0xb8(t8)
	
	lb		a0, 0xba(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	or		a0, a0, t4
	sb		a0, 0xb9(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0xa1(t8)
	lb		a0, 0xba(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0xa1(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0xa0(t8)
	lb		a0, 0xb9(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0xa0(t8)
1:
    
    b       lvl_req_set //make additional 5 times lvl_req
    nop

dll_wrdqs5_add:
    daddu   s6, s6, 0x1
    ld      a0, 0xd8(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0xd8(t8)
	
	lb		a0, 0xda(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	or		a0, a0, t4
	sb		a0, 0xd9(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0xc1(t8)
	lb		a0, 0xda(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0xc1(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0xc0(t8)
	lb		a0, 0xd9(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0xc0(t8)
1:
    
    b       lvl_req_set //make additional 5 times lvl_req
    nop

dll_wrdqs6_add:
    daddu   s6, s6, 0x1
    ld      a0, 0xf8(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0xf8(t8)
	
	lb		a0, 0xfa(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	or		a0, a0, t4
	sb		a0, 0xf9(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0xe1(t8)
	lb		a0, 0xfa(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0xe1(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0xe0(t8)
	lb		a0, 0xf9(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0xe0(t8)
1:
    
    b       lvl_req_set //make additional 5 times lvl_req
    nop

dll_wrdqs7_add:
    daddu   s6, s6, 0x1
    ld      a0, 0x118(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x118(t8)
	
	lb		a0, 0x11a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	or		a0, a0, t4
	sb		a0, 0x119(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x101(t8)
	lb		a0, 0x11a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x101(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x100(t8)
	lb		a0, 0x119(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x100(t8)
1:
    
    b       lvl_req_set //make additional 5 times lvl_req
    nop

dll_wrdqs8_add:
    daddu   s6, s6, 0x1
    ld      a0, 0x138(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x138(t8)
	
	lb		a0, 0x13a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	or		a0, a0, t4
	sb		a0, 0x139(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x121(t8)
	lb		a0, 0x13a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x121(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x120(t8)
	lb		a0, 0x139(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x120(t8)
1:
    
    b       lvl_req_set //make additional 5 times lvl_req
    nop

2221:
#endif

    dli     t3, 0x1
	dli		s6, 0x3
    b       lvl_req_set
    nop

/* 7.2 start from all slice got 0, until all 1 found */

/* a2 is used to indicate whether adjust happened */
lvl_resp_set:

    //jr      ra
    //nop

    //dli     s6, 0x1
lvl_resp_0_set1:
    dli     s7, 0x0
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0100000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 56
    beq     a0, $0, dll_wrdqs_0_add2
    nop
lvl_resp_1_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000000001
    and     a0, a0, t4
    beq     a0, $0, dll_wrdqs_1_add2
    nop
lvl_resp_2_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000000100
    and     a0, a0, t4
    dsrl    a0, a0, 8
    beq     a0, $0, dll_wrdqs_2_add2
    nop
lvl_resp_3_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000010000
    and     a0, a0, t4
    dsrl    a0, a0, 16
    beq     a0, $0, dll_wrdqs_3_add2
    nop
lvl_resp_4_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000001000000
    and     a0, a0, t4
    dsrl    a0, a0, 24
    beq     a0, $0, dll_wrdqs_4_add2
    nop
lvl_resp_5_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000100000000
    and     a0, a0, t4
    dsrl    a0, a0, 32
    beq     a0, $0, dll_wrdqs_5_add2
    nop
lvl_resp_6_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000010000000000
    and     a0, a0, t4
    dsrl    a0, a0, 40
    beq     a0, $0, dll_wrdqs_6_add2
    nop
lvl_resp_7_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0001000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 48
    beq     a0, $0, dll_wrdqs_7_add2
    nop
lvl_resp_8_set1:
/* identify wheather there is ecc slice */
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0100000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 56
    beq     a0, $0, dll_wrdqs_8_add2
    nop
1:

    beqz	s7, 1f
    nop
	dli 	s6, 0x6
	b		lvl_req_set
	nop
1:
    //jr      ra
    //nop

#ifdef FILTER_CHECK
check_5_slice0:
	bnez	t0, check_5_slice1
	nop
//	PRINTSTR("\r\ncheck slice 0\r\n")
//	move	a0, s6
//	bal		hexserial
//	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x38
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	b		lvl_req_set
	nop
1:
    dli     t1, 0x38
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	dli		s6, 0x6
	daddu	t0, t0, 0x1
check_5_slice1:
	bne		t0, 1, check_5_slice2
	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x58
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	b		lvl_req_set
	nop
1:
    dli     t1, 0x58
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	dli		s6, 0x6
	daddu	t0, t0, 0x1
check_5_slice2:
	bne		t0, 2, check_5_slice3
	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x78
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	b		lvl_req_set
	nop
1:
    dli     t1, 0x78
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	dli		s6, 0x6
	daddu	t0, t0, 0x1
check_5_slice3:
	bne		t0, 3, check_5_slice4
	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x98
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	b		lvl_req_set
	nop
1:
    dli     t1, 0x98
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	dli		s6, 0x6
	daddu	t0, t0, 0x1
check_5_slice4:
	bne		t0, 4, check_5_slice5
	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0xb8
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	b		lvl_req_set
	nop
1:
    dli     t1, 0xb8
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	dli		s6, 0x6
	daddu	t0, t0, 0x1
check_5_slice5:
	bne		t0, 5, check_5_slice6
	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0xd8
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	b		lvl_req_set
	nop
1:
    dli     t1, 0xd8
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	dli		s6, 0x6
	daddu	t0, t0, 0x1
check_5_slice6:
	bne		t0, 6, check_5_slice7
	nop
//	PRINTSTR("\r\ncheck slice 6\r\n")
//	move	a0, s6
//	bal		hexserial
//	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0xf8
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	b		lvl_req_set
	nop
1:
    dli     t1, 0xf8
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	dli		s6, 0x6
	daddu	t0, t0, 0x1
check_5_slice7:
	bne		t0, 7, check_5_slice8
	nop
//	PRINTSTR("\r\ncheck slice 7\r\n")
//	move	a0, s6
//	bal		hexserial
//	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x118
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	b		lvl_req_set
	nop
1:
    dli     t1, 0x118
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	dli		s6, 0x6
	daddu	t0, t0, 0x1
check_5_slice8:
/* identify wheather there is ecc slice */
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, check_5_end //when rd_after_write enabled, the 9th device may don't need leveling
    nop
1:
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x138
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	b		lvl_req_set
	nop
1:
    dli     t1, 0x138
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x2(t1)
	dli		s6, 0x0
	daddu	t0, t0, 0x1
check_5_end:
PRINTSTR("\r\nfiltering check success\r\n")
#endif

/* 8. All 1 found, set params according to wrdqs */

//    GET_DIMM_TYPE
//    beqz    a1, 81f
//    nop

/* adjust wrdqs carefully */
#if 0   //def  DEBUG_DDR_PARAM   //print registers
    PRINTSTR("\r\nThe MC param before carefully adjust is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, 0x900000000ff00000
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
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

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
#endif
wrdqs_adjust:
#if 1
#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrdqs around 0x00 carefully adjust begin\r\n")
#endif
/* identify wheather there is ecc slice */
    li      t0, 0x9
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
    dli     a2, 0x08
    dli     a3, 0x78
//    dli     t0, 0x8
    dli     t1, 0x018
    or      t1, t1, t8
wrdqs_adjust_loop00:
    subu    t0, t0, 0x1
    beq     t0, $0, 1f
    nop
    daddu   t1, t1, 0x20
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000ff0000
    and     a0, a0, t4
    dsrl    a0, a0, 16
    bgtu    a0, a2, wrdqs_adjust_loop00
    nop
    bltu    a0, a3, wrdqs_adjust_loop00
    nop
    dli     t4, 0x70
    and     t4, t4, a0
    beqz    t4, wrdqs_set_08
    nop
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffff00ffff
    and     a0, a0, t4
    dli     t4, 0x780000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
	
	lb		a0, 0x2(t1) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x1(t1)
	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x2(t1)   
	blt		a0, 0x40, 22f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0	
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
22:

	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x1(t1)
	blt		a0, 0x40, 22f   //if dll_dq less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
22:

    b       wrdqs_adjust_loop00
    nop

wrdqs_set_08:
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffff00ffff
    and     a0, a0, t4
    dli     t4, 0x080000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
	
	lb		a0, 0x2(t1) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x1(t1)
	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x2(t1)   
	blt		a0, 0x40, 22f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0	
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
22:

	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x1(t1)
	blt		a0, 0x40, 22f   //if dll_dq less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
22:
    b       wrdqs_adjust_loop00
    nop

#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrdqs around 0x00 carefully adjust end\r\n")
#endif
1:
#endif
#if 1
#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrdqs around 0x20 carefully adjust begin\r\n")
#endif
/* identify wheather there is ecc slice */
    li      t0, 0x9
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
    dli     a2, 0x28
    dli     a3, 0x18
//    dli     t0, 0x8
    dli     t1, 0x018
    or      t1, t1, t8
wrdqs_adjust_loop20:
    subu    t0, t0, 0x1
    beq     t0, $0, 1f
    nop
    daddu   t1, t1, 0x20
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000ff0000
    and     a0, a0, t4
    dsrl    a0, a0, 16
    bgtu    a0, a2, wrdqs_adjust_loop20
    nop
    bltu    a0, a3, wrdqs_adjust_loop20
    nop
    dli     t4, 0x20
    bltu    a0, t4, wrdqs_set_18
    nop
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffff00ffff
    and     a0, a0, t4
    dli     t4, 0x280000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
	
	lb		a0, 0x2(t1) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x1(t1)
	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x2(t1)   
	blt		a0, 0x40, 22f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0	
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
22:

	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x1(t1)
	blt		a0, 0x40, 22f   //if dll_dq less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
22:
    b       wrdqs_adjust_loop20
    nop

wrdqs_set_18:
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffff00ffff
    and     a0, a0, t4
    dli     t4, 0x180000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
	
	lb		a0, 0x2(t1) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x1(t1)
	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x2(t1)   
	blt		a0, 0x40, 22f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0	
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
22:

	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x1(t1)
	blt		a0, 0x40, 22f   //if dll_dq less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
22:
    b       wrdqs_adjust_loop20
    nop

1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrdqs around 0x40 carefully adjust end\r\n")
#endif
#endif
#if 1
#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrdqs around 0x40 carefully adjust begin\r\n")
#endif
/* identify wheather there is ecc slice */
    li      t0, 0x9
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
    dli     a2, 0x48
    dli     a3, 0x38
//    dli     t0, 0x8
    dli     t1, 0x018
    or      t1, t1, t8
wrdqs_adjust_loop40:
    subu    t0, t0, 0x1
    beq     t0, $0, 1f
    nop
    daddu   t1, t1, 0x20
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000ff0000
    and     a0, a0, t4
    dsrl    a0, a0, 16
    bgtu    a0, a2, wrdqs_adjust_loop40
    nop
    bltu    a0, a3, wrdqs_adjust_loop40
    nop
    dli     t4, 0x40
    bltu    a0, t4, wrdqs_set_3a
    nop
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffff00ffff
    and     a0, a0, t4
    dli     t4, 0x480000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
	
	lb		a0, 0x2(t1) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x1(t1)
	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x2(t1)   
	blt		a0, 0x40, 22f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0	
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
22:

	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x1(t1)
	blt		a0, 0x40, 22f   //if dll_dq less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
22:
    b       wrdqs_adjust_loop40
    nop

wrdqs_set_3a:
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffff00ffff
    and     a0, a0, t4
    dli     t4, 0x380000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
	
	lb		a0, 0x2(t1) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x1(t1)
	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x2(t1)   
	blt		a0, 0x40, 22f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0	
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
22:

	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x1(t1)
	blt		a0, 0x40, 22f   //if dll_dq less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
22:
    b       wrdqs_adjust_loop40
    nop

1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrdqs around 0x40 carefully adjust end\r\n")
#endif
#endif
#if 1
#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrdqs around 0x60 carefully adjust begin\r\n")
#endif
/* identify wheather there is ecc slice */
    li      t0, 0x9
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
    dli     a2, 0x68
    dli     a3, 0x58
//    dli     t0, 0x8
    dli     t1, 0x018
    or      t1, t1, t8
wrdqs_adjust_loop60:
    subu    t0, t0, 0x1
    beq     t0, $0, 1f
    nop
    daddu   t1, t1, 0x20
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000ff0000
    and     a0, a0, t4
    dsrl    a0, a0, 16
    bgtu    a0, a2, wrdqs_adjust_loop60
    nop
    bltu    a0, a3, wrdqs_adjust_loop60
    nop
    dli     t4, 0x60
    bltu    a0, t4, wrdqs_set_5a
    nop
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffff00ffff
    and     a0, a0, t4
    dli     t4, 0x680000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
	
	lb		a0, 0x2(t1) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x1(t1)
	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x2(t1)   
	blt		a0, 0x40, 22f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0	
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
22:

	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x1(t1)
	blt		a0, 0x40, 22f   //if dll_dq less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
22:
    b       wrdqs_adjust_loop60
    nop

wrdqs_set_5a:
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffff00ffff
    and     a0, a0, t4
    dli     t4, 0x580000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
	
	lb		a0, 0x2(t1) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x1(t1)
	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x2(t1)   
	blt		a0, 0x40, 22f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0	
	sb		a0, 0x1(t1)
	daddu	t1, t1, 0x18
22:

	dsubu	t1, t1, 0x18
	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
	lb		a0, 0x1(t1)
	blt		a0, 0x40, 22f   //if dll_dq less then a3 then set 1 else set 0
	nop
	dsubu	t1, t1, 0x18
	li		a0, 0x0
	sb		a0, 0x0(t1)
	daddu	t1, t1, 0x18
22:
    b       wrdqs_adjust_loop60
    nop

#ifdef LVL_DEBUG
    PRINTSTR("\r\nwrdqs around 0x60 carefully adjust end\r\n")
#endif
1:
#endif

#if 0   //def  DEBUG_DDR_PARAM   //print registers
    PRINTSTR("\r\nThe MC param after carefully adjust is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, 0x900000000ff00000
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
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

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
#endif
81:

#if 1
/* 8.1 adjust wrdata */

/* t0 is used to indicate 8 slices */
/* identify wheather there is ecc slice */
    li      t0, 0x8
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
    dli     a2, 0x20
    dli     t1, 0x018
    or      t1, t1, t8
dll_wrdata_set:
    daddu   t1, t1, 0x20
    ld      a0, 0x0(t1)
    move    a1, a0
    dli     t4, 0x0000000000ff0000
    and     a1, a1, t4
    dsrl    a1, a1, 16
    bltu    a1, a2, dll_wrdata_add60
    nop
    b       dll_wrdata_sub20
    nop

/* add 0x60 when wrdqs is smaller than 0x20 */
dll_wrdata_add60:
    daddu   a1, a1, 0x60
    dsll    a1, a1, 8
    dli     t4, 0xffffffffffff00ff
    and     a0, a0, t4
    daddu   a0, a0, a1
    sd      a0, 0x0(t1)
    subu    t0, t0, 0x1
    beq     t0, $0, wrdqs_lt_half_set
    nop
    b       dll_wrdata_set
    nop

/* sub 0x20 when wrdqs is bigger than 0x20 */
dll_wrdata_sub20:
    dsubu    a1, a1, 0x20
    dsll    a1, a1, 8
    dli     t4, 0xffffffffffff00ff
    and     a0, a0, t4
    daddu    a0, a0, a1
    sd      a0, 0x0(t1)
    subu    t0, t0, 0x1
    beq     t0, $0, wrdqs_lt_half_set
    nop
    b       dll_wrdata_set
    nop

/* 8.2 adjust wrdqs_lt_half */
wrdqs_lt_half_set:
/* identify wheather there is ecc slice */
    li      t0, 0x8
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
    dli     a2, 0x40
//    dli     t0, 0x8
    dli     t1, 0x018
    or      t1, t1, t8
wrdqs_lt_half_loop:
    beq     t0, $0, wrdq_lt_half_set
    nop
    daddu    t1, t1, 0x20
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000ff0000
    and     a0, a0, t4
    dsrl    a0, a0, 16
    bltu    a0, a2, wrdqs_lt_half_set1
    subu    t0, t0, 0x1
    b       wrdqs_lt_half_set0
    nop
wrdqs_lt_half_set0:
    dsubu    t2, t1, 0x18
    ld      a0, 0x0(t2)
    dli     t4, 0xffffffffffff00ff
    and     a0, a0, t4
    sd      a0, 0x0(t2)
    b       wrdqs_lt_half_loop
    nop

wrdqs_lt_half_set1:
    dsubu   t2, t1, 0x18
    ld      a0, 0x0(t2)
    dli     t4, 0xffffffffffff00ff
    and     a0, a0, t4
    ori     a0, a0, 0x100
    sd      a0, 0x0(t2)
    b       wrdqs_lt_half_loop
    nop

/* 8.3 adjust wrdq_lt_half */
wrdq_lt_half_set:
/* identify wheather there is ecc slice */
    li      t0, 0x8
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
    dli     a2, 0x40
    dli     t5, 0x0101000000000000 // meiyong keshan?
    dli     t6, 0x0000000001010000 // tongshang
//    li     t0, 0x8
    dli     t1, 0x018
    or      t1, t1, t8
wrdq_lt_half_loop:
    beq     t0, $0, wrdq_lt_half_test
    nop
    daddu    t1, t1, 0x20
    ld      a0, 0x0(t1)
    dli     t4, 0x000000000000ff00
    and     a0, a0, t4
    dsrl    a0, a0, 8
    bltu    a0, a2, wrdq_lt_half_set1
    subu    t0, t0, 0x1
    b       wrdq_lt_half_set0
    nop
wrdq_lt_half_set0:
    dsubu   t2, t1, 0x18
    ld      a0, 0x0(t2)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    sd      a0, 0x0(t2)

/*    daddu   t2, t2, 0x10
    ld      a0, 0x0(t2)
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x0(t2)*/

    b       wrdq_lt_half_loop
    nop
wrdq_lt_half_set1:
    dsubu   t2, t1, 0x18
    ld      a0, 0x0(t2)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    ori     a0, a0, 0x1
    sd      a0, 0x0(t2)

    b       wrdq_lt_half_loop
    nop
#endif

wrdq_lt_half_test:
    GET_DIMM_TYPE
    bnez    a1, rdimm_wrdq_lt_half_test
    nop
    li      t0, 0x7 //only loop 7 times
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
    dli     t1, 0x20
    dli     t2, 0x0
    or      t1, t1, t8

wrdq_lt_half_test_loop:
    daddu   t2, t2, 0x1
    bgt     t2, t0, record_slice_num
    nop
    lb      a0, 0x0(t1)
    daddu   t1, t1, 0x20
    lb      a1, 0x0(t1)
    beqz     a0, wrdq_lt_half_test_loop
    nop
    beqz     a1, record_slice_num
    nop
    b       wrdq_lt_half_test_loop
    nop

record_slice_num:
    move    t3, t2 //the slice number save in t3
    beq     t3, 0x8, first_slice_wrdq_lt_half_test
    nop

wrdq_clkdelay_set:
//    li      t0, 0x7 //only loop 7 times
    dli     t1, 0x30
    dli     t2, 0x0
    or      t1, t1, t8
wrdq_clkdelay_set_loop:
    daddu   t2, t2, 0x1
    bgt     t2, t0, first_slice_wrdq_lt_half_test
    nop
    daddu   t1, t1, 0x20
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
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
    b       wrdq_clkdelay_set_loop
    nop

first_slice_wrdq_lt_half_test:
    dli     t1, 0x20
    or      t1, t1, t8
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
    dli     t1, 0x120
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    dsubu   t1, t1, 0xa0
    lb      a1, 0x0(t1)
    daddu   t2, t2, 0x1
    beqz    a0, rdimm_wrdq_lt_half_test_loop_3210
    nop
    beqz    a1, rdimm_record_slice_num_83210
    nop
    b       rdimm_wrdq_lt_half_test_loop_3210
    nop


rdimm_wrdq_lt_half_test_3210:
    li      t0, 0x3
    dli     t1, 0x80
    dli     t2, 0x0
    or      t1, t1, t8
    
rdimm_wrdq_lt_half_test_loop_3210:
    daddu   t2, t2, 0x1
    bgt     t2, t0, rdimm_wrdq_lt_half_test_4567
    nop
    lb      a0, 0x0(t1)
    dsubu   t1, t1, 0x20
    lb      a1, 0x0(t1)
    beqz    a0, rdimm_wrdq_lt_half_test_loop_3210
    nop
    beqz    a1, rdimm_record_slice_num_3210
    nop
    b       rdimm_wrdq_lt_half_test_loop_3210
    nop

rdimm_record_slice_num_3210:
rdimm_record_slice_num_83210:
    move    t3, t2 

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
    dli     t1, 0x130
    or      t1, t1, t8
    dli     t2, 0x0
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
    li      t0, 0x3 
    dli     t1, 0x90
    dli     t2, 0x0
    or      t1, t1, t8
rdimm_wrdq_clkdelay_set_loop_3210:
1:
    daddu   t2, t2, 0x1
    bgt     t2, t0, rdimm_wrdq_lt_half_test_4567
    nop
    dsubu   t1, t1, 0x20
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
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
    b       1b
    nop


rdimm_wrdq_lt_half_test_4567:
    li      t0, 0x3 
    dli     t1, 0xa0
    dli     t2, 0x0
    or      t1, t1, t8

rdimm_wrdq_lt_half_test_loop_4567:
    daddu   t2, t2, 0x1
    bgt     t2, t0, slice_8_wrdq_lt_half_test
    nop
    lb      a0, 0x0(t1)
    daddu   t1, t1, 0x20
    lb      a1, 0x0(t1)
    beqz    a0, rdimm_wrdq_lt_half_test_loop_4567
    nop
    beqz    a1, rdimm_record_slice_num_4567
    nop
    b       rdimm_wrdq_lt_half_test_loop_4567
    nop

rdimm_record_slice_num_4567:
    move    t3, t2 //the slice number save in t3

rdimm_wrdq_clkdelay_set_4567:
    li      t0, 0x3 //only loop 7 times
    dli     t1, 0xb0
    dli     t2, 0x0
    or      t1, t1, t8
rdimm_wrdq_clkdelay_set_loop_4567:
    daddu   t2, t2, 0x1
    bgt     t2, t0, slice_8_wrdq_lt_half_test
    nop
    daddu   t1, t1, 0x20
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
    dli     t4, 0xffffff00ffffffff
    and     a0, a0, t4
    dli     t4, 0x0000000100000000
    or      a0, a0, t4
    sd      a0, 0x0(t1)
    b       rdimm_wrdq_clkdelay_set_loop_4567
    nop

slice_8_wrdq_lt_half_test:
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, slice_3_wrdq_lt_half_test
    nop
    dli     t1, 0x120
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000000000ff
    and     a0, a0, t4
    bnez    a0, rdimm_trddata_tphywrdata_sub
    nop
    b       slice_4_wrdq_lt_half_test
    nop

slice_3_wrdq_lt_half_test:
    dli     t1, 0x80
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000000000ff
    and     a0, a0, t4
    bnez    a0, rdimm_trddata_tphywrdata_sub
    nop
    
slice_4_wrdq_lt_half_test:
    dli     t1, 0xa0
    or      t1, t1, t8
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

dll_wrdqs_0_add1:
    ld      a0, 0x38(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x38(t8)

#if 1
	lb		a0, 0x3a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x39(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x21(t8)
	lb		a0, 0x3a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x21(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x20(t8)
	lb		a0, 0x39(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x20(t8)
1:
#endif	
    dli     s7, 0x1
    b       lvl_resp_1_set0 // 
    nop

dll_wrdqs_1_add1:
    daddu   s6, s6, 0x1
    ld      a0, 0x58(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x58(t8)
	
	lb		a0, 0x5a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x59(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x41(t8)
	lb		a0, 0x5a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x41(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x40(t8)
	lb		a0, 0x59(t8)
	blt		a0, a3, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x40(t8)
1:
    
    dli     s7, 0x1
	b       lvl_resp_2_set0 // 
    nop

dll_wrdqs_2_add1:
    daddu   s6, s6, 0x1
    ld      a0, 0x78(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x78(t8)
	
	lb		a0, 0x7a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x79(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x61(t8)
	lb		a0, 0x7a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x61(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x60(t8)
	lb		a0, 0x79(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x60(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_3_set0 // 
    nop

dll_wrdqs_3_add1:
    daddu   s6, s6, 0x1
    ld      a0, 0x98(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x98(t8)
	
	lb		a0, 0x9a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x99(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x81(t8)
	lb		a0, 0x9a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x81(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x80(t8)
	lb		a0, 0x99(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x80(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_4_set0 // 
    nop

dll_wrdqs_4_add1:
    daddu   s6, s6, 0x1
    ld      a0, 0xb8(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0xb8(t8)
	
	lb		a0, 0xba(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0xb9(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0xa1(t8)
	lb		a0, 0xba(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0xa1(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0xa0(t8)
	lb		a0, 0xb9(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0xa0(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_5_set0 // 
    nop

dll_wrdqs_5_add1:
    daddu   s6, s6, 0x1
    ld      a0, 0xd8(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0xd8(t8)
	
	lb		a0, 0xda(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0xd9(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0xc1(t8)
	lb		a0, 0xda(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0xc1(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0xc0(t8)
	lb		a0, 0xd9(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0xc0(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_6_set0 // 
    nop

dll_wrdqs_6_add1:
    daddu   s6, s6, 0x1
    ld      a0, 0xf8(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0xf8(t8)
	
	lb		a0, 0xfa(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0xf9(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0xe1(t8)
	lb		a0, 0xfa(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0xe1(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0xe0(t8)
	lb		a0, 0xf9(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0xe0(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_7_set0 // 
    nop

dll_wrdqs_7_add1:
    daddu   s6, s6, 0x1
    ld      a0, 0x118(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x118(t8)
	
	lb		a0, 0x11a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x119(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x101(t8)
	lb		a0, 0x11a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x101(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x100(t8)
	lb		a0, 0x119(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x100(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_8_set0 // 
    nop

dll_wrdqs_8_add1:
    daddu   s6, s6, 0x1
    ld      a0, 0x138(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x138(t8)
	
	lb		a0, 0x13a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x139(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x121(t8)
	lb		a0, 0x13a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x121(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x120(t8)
	lb		a0, 0x139(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x120(t8)
1: 
    dli     s7, 0x1
    b       lvl_req_set // 
    nop

#if 0
dll_wrdqs_0_add1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 0, slice 0 add\r\n")
#endif
    dli     t1, 0x038
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu   a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_1_set0
    dli     s7, 0x1 
dll_wrdqs_1_add1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 0, slice 1 add\r\n")
#endif
    dli     t1, 0x058
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_2_set0
    dli     s7, 0x1
dll_wrdqs_2_add1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 0, slice 2 add\r\n")
#endif
    dli     t1, 0x078
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_3_set0
    dli     s7, 0x1
dll_wrdqs_3_add1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 0, slice 3 add\r\n")
#endif
    dli     t1, 0x098
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_4_set0
    dli     s7, 0x1
dll_wrdqs_4_add1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 0, slice 4 add\r\n")
#endif
    dli     t1, 0x0b8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_5_set0
    dli     s7, 0x1
dll_wrdqs_5_add1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 0, slice 5 add\r\n")
#endif
    dli     t1, 0x0d8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_6_set0
    dli     s7, 0x1
dll_wrdqs_6_add1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 0, slice 6 add\r\n")
#endif
    dli     t1, 0x0f8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_7_set0
    dli     s7, 0x1
dll_wrdqs_7_add1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 0, slice 7 add\r\n")
#endif
    dli     t1, 0x118
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_8_set0
    dli     s7, 0x1
dll_wrdqs_8_add1:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 0, slice 8 add\r\n")
#endif
    dli     t1, 0x138
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_req_set
    dli     s7, 0x1
#endif

dll_wrdqs_0_add2:
    ld      a0, 0x38(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x38(t8)

#if 1
	lb		a0, 0x3a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x39(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x21(t8)
	lb		a0, 0x3a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x21(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x20(t8)
	lb		a0, 0x39(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x20(t8)
1:
#endif	
    dli     s7, 0x1
    b       lvl_resp_1_set1 // 
    nop

dll_wrdqs_1_add2:
    daddu   s6, s6, 0x1
    ld      a0, 0x58(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x58(t8)
	
	lb		a0, 0x5a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x59(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x41(t8)
	lb		a0, 0x5a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x41(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x40(t8)
	lb		a0, 0x59(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x40(t8)
1:
    
    dli     s7, 0x1
	b       lvl_resp_2_set1 // 
    nop

dll_wrdqs_2_add2:
    daddu   s6, s6, 0x1
    ld      a0, 0x78(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x78(t8)
	
	lb		a0, 0x7a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x79(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x61(t8)
	lb		a0, 0x7a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x61(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x60(t8)
	lb		a0, 0x79(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x60(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_3_set1 // 
    nop

dll_wrdqs_3_add2:
    daddu   s6, s6, 0x1
    ld      a0, 0x98(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x98(t8)
	
	lb		a0, 0x9a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x99(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x81(t8)
	lb		a0, 0x9a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x81(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x80(t8)
	lb		a0, 0x99(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x80(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_4_set1 // 
    nop

dll_wrdqs_4_add2:
    daddu   s6, s6, 0x1
    ld      a0, 0xb8(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0xb8(t8)
	
	lb		a0, 0xba(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0xb9(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0xa1(t8)
	lb		a0, 0xba(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0xa1(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0xa0(t8)
	lb		a0, 0xb9(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0xa0(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_5_set1 // 
    nop

dll_wrdqs_5_add2:
    daddu   s6, s6, 0x1
    ld      a0, 0xd8(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0xd8(t8)
	
	lb		a0, 0xda(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0xd9(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0xc1(t8)
	lb		a0, 0xda(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0xc1(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0xc0(t8)
	lb		a0, 0xd9(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0xc0(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_6_set1 // 
    nop

dll_wrdqs_6_add2:
    daddu   s6, s6, 0x1
    ld      a0, 0xf8(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0xf8(t8)
	
	lb		a0, 0xfa(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0xf9(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0xe1(t8)
	lb		a0, 0xfa(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0xe1(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0xe0(t8)
	lb		a0, 0xf9(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0xe0(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_7_set1 // 
    nop

dll_wrdqs_7_add2:
    daddu   s6, s6, 0x1
    ld      a0, 0x118(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x118(t8)
	
	lb		a0, 0x11a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x119(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x101(t8)
	lb		a0, 0x11a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x101(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x100(t8)
	lb		a0, 0x119(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x100(t8)
1:
    
    dli     s7, 0x1
    b       lvl_resp_8_set1 // 
    nop

dll_wrdqs_8_add2:
    daddu   s6, s6, 0x1
    ld      a0, 0x138(t8)
    dli     t4, 0x010000
    daddu   a0, a0, t4
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x138(t8)
	
	lb		a0, 0x13a(t8) // set dll_wrdata
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x139(t8)
	li		a0, 0x1		// set wrdqs_lt_half
	sb		a0, 0x121(t8)
	lb		a0, 0x13a(t8)   
	blt		a0, 0x40, 1f   //if dll_dqs less then a3 then set 1 else set 0
	nop
	li		a0, 0x0	
	sb		a0, 0x121(t8)
1:

	li		a0, 0x1		// set  wrdq_lt_half
	sb		a0, 0x120(t8)
	lb		a0, 0x139(t8)
	blt		a0, 0x40, 1f   //if dll_dq less then a3 then set 1 else set 0
	nop
	li		a0, 0x0
	sb		a0, 0x120(t8)
1: 
    dli     s7, 0x1
    b       lvl_req_set // 
    nop

#if 0
dll_wrdqs_0_add2:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 1, slice 0 add\r\n")
#endif
    dli     t1, 0x038
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_1_set1
    dli     s7, 0x1 
dll_wrdqs_1_add2:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 1, slice 1 add\r\n")
#endif
    dli     t1, 0x058
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_2_set1
    dli     s7, 0x1
dll_wrdqs_2_add2:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 1, slice 2 add\r\n")
#endif
    dli     t1, 0x078
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_3_set1
    dli     s7, 0x1
dll_wrdqs_3_add2:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 1, slice 3 add\r\n")
#endif
    dli     t1, 0x098
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_4_set1
    dli     s7, 0x1
dll_wrdqs_4_add2:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 1, slice 4 add\r\n")
#endif
    dli     t1, 0x0b8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_5_set1
    dli     s7, 0x1
dll_wrdqs_5_add2:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 1, slice 5 add\r\n")
#endif
    dli     t1, 0x0d8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_6_set1
    dli     s7, 0x1
dll_wrdqs_6_add2:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 1, slice 6 add\r\n")
#endif
    dli     t1, 0x0f8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_7_set1
    dli     s7, 0x1
dll_wrdqs_7_add2:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 1, slice 7 add\r\n")
#endif
    dli     t1, 0x118
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_resp_8_set1
    dli     s7, 0x1
dll_wrdqs_8_add2:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all write resp got a 1, slice 8 add\r\n")
#endif
    dli     t1, 0x138
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x10000
    dli     t4, 0xffffffffff7fffff
    and     a0, a0, t4
    sd      a0, 0x0(t1)
    b       lvl_req_set
    dli     s7, 0x1
#endif

gate_leveling:
#ifdef	EDGES_CHECK_DEBUG //save the init para of rd_oe and trddata
	dli		t1, 0x28
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
#endif
#if 1 //3a3000 new
//    PRINTSTR("\r\nset cs_zq to be same with cs_enable\r\n")
    lb      a0, 0x169(t8)
    sb      a0, 0x16a(t8)

reset_init_start_new:
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

wait_init_done_new:
    dli     t1, 0x160
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000ff000000
    and     a0, a0, t4
    beqz    a0, wait_init_done_new
    nop

reset_init_start_new2:
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

wait_init_done_new2:
    dli     t1, 0x160
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000ff000000
    and     a0, a0, t4
    beqz    a0, wait_init_done_new2
    nop

#endif
    PRINTSTR("\r\nwrite leveling finish and gate leveling begin\r\n")
#ifdef  PRINT_DDR_LEVELING   //print registers
    PRINTSTR("\r\nThe MC param after write leveling is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, 0x900000000ff00000
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
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

    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
#endif

/* identify wheather there is ecc slice */
    li      t0, 0x8
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
    dli     t1, 0x018
    or      t1, t1, t8
dll_gate_set0:
    daddu    t1, t1, 0x20
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
#ifdef DDR_DLL_BYPASS
    dli     t4, 0x0000000000000080
    or      a0, a0, t4
#endif
    sd      a0, 0x0(t1)
    subu    t0, t0, 0x1 
    bnez    t0, dll_gate_set0 
    nop
 
glvl_mode_set10:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    ori     a0, a0, 0x2
    sd      a0, 0x0(t1)

    dli     a1, 0x1
glvl_ready_sampling:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000ff0000000000
    and     a0, a0, t4
    dsrl    a0, a0, 40
    bne     a0, a1, glvl_ready_sampling
    nop

#ifdef SIGNAL_DEPICT_DEBUG
	PRINTSTR("\r\nthe signal depict begin:\r\n")
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
	dli		t0, 0x8
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
	dli		t4, 0x1
	and		a0, a0, t4
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
	dli		t4, 0x1
	and		a0, a0, t4
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
    dli     a0, 0x1
    sb      a0, 0x1(t4)
    dli		a0, 0x0
    sb      a0, 0x1(t4)
    dli     a0, 0x1
    sb      a0, 0x1(t4)
    dli		a0, 0x0
    sb      a0, 0x1(t4)
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
	dli		t4, 0x1
	and		a0, a0, t4
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
#ifdef DDR_DLL_BYPASS
	lw		t4, 0x4(t8)
	daddu	t4, t4, 0x2
	blt		a0, t4, 1f
	nop
	dli		a0, 0x80
	sb		a0, 0x10(t1)
	dli		a0, 0x0
	b		2f
	nop
1:
	ori		a0, 0x80
	sb		a0, 0x10(t1)
2:
#endif
	bnez	a0, 1f
	nop
	lb		a0, 0x1c0(t8)
	daddu	a0, a0, 0x1
	sb		a0, 0x1c0(t8)
1:
#endif
#if 0
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
#if 0
	lb		a0, 0x11(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x11(t1)
	bne		a0, 0x0,1f
	nop
	lb		a0, 0x1c0(t8)
	daddu	a0, a0, 0x1
	sb		a0, 0x1c0(t8)
1:
#endif
#if 0
	lb		a0, 0x12(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x12(t1)
	bne		a0, 0x0,1f
	nop
	lb		a0, 0x1c0(t8)
	daddu	a0, a0, 0x1
	sb		a0, 0x1c0(t8)
1:
#endif
	b		t_glvl_req_set
	nop

signal_depict_end:
//identify wheather there is ecc slice
	li      t0, 0x8
	dli     t1, 0x250
	or      t1, t1, t8
	lb      a0, 0x2(t1)
	dli     t1, 0x1
	and     a0, a0, t1
	bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
	nop
	daddu   t0, t0, 0x1
1:
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
	
//identify wheather there is ecc slice
/*	li      t0, 0x8
	dli     t1, 0x250
	or      t1, t1, t8
	lb      a0, 0x2(t1)
	dli     t1, 0x1
	and     a0, a0, t1
	bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
	nop
	daddu   t0, t0, 0x1
1:
	dli		t1, 0x30
	or		t1, t1, t8
reset_odt_oe:
	dli		a0, 0x2
	sb		a0, 0x2(t1)
	sb		a0, 0x3(t1)
	daddu	t1, t1, 0x20
	dsubu	t0, t0, 0x1
	bnez	t0, reset_odt_oe
	nop*/

	PRINTSTR("\r\n")
#endif

    dli     t3, 0x0
	dli		t0, 0x0
	dli		s6, 0x0 //
glvl_req_set:
//	PRINTSTR("\r\n req")
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffff00ff
    and     a0, a0, t4
    ori     a0, a0, 0x100
    sd      a0, 0x0(t1)
    and     a0, a0, t4
    sd      a0, 0x0(t1)

    dli     a1, 0x1

glvl_done_sampling:
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00ff000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 48
    bne     a0, a1, glvl_done_sampling
    nop

    beq     t3, 0x1, glvl_resp_set
    nop 

#if 1
//  dli     s6, 0x1
glvl_resp_0_set0:
    dli     s7, 0x0
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0300000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 56
    bne     a0, $0, dll_gate_0_add0
    nop
glvl_resp_1_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000000003
    and     a0, a0, t4
    bne     a0, $0, dll_gate_1_add0
    nop
glvl_resp_2_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000000300
    and     a0, a0, t4
    dsrl    a0, a0, 8
    bne     a0, $0, dll_gate_2_add0
    nop
glvl_resp_3_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000030000
    and     a0, a0, t4
    dsrl    a0, a0, 16
    bne     a0, $0, dll_gate_3_add0
    nop
glvl_resp_4_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000003000000
    and     a0, a0, t4
    dsrl    a0, a0, 24
    bne     a0, $0, dll_gate_4_add0
    nop
glvl_resp_5_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000300000000
    and     a0, a0, t4
    dsrl    a0, a0, 32
    bne     a0, $0, dll_gate_5_add0
    nop
glvl_resp_6_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000030000000000
    and     a0, a0, t4
    dsrl    a0, a0, 40
    bne     a0, $0, dll_gate_6_add0
    nop
glvl_resp_7_set0:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0003000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 48
    bne     a0, $0, dll_gate_7_add0
    nop
glvl_resp_8_set0:
/* identify wheather there is ecc slice */
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0300000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 56
    bne     a0, $0, dll_gate_8_add0
    nop

1:
    beq     s7, 0x1, glvl_req_set
    nop
#endif
#ifdef  PRINT_DDR_LEVELING   //print registers
    PRINTSTR("\r\nThe MC param after gate leveling 1 to 0 is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, 0x900000000ff00000
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
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

    
	daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
    nop
#ifdef EDGES_CHECK_DEBUG	
	PRINTSTR("\r\nThe above is NO.")
	move	a0, s6
	bal		hexserial
	nop
	PRINTSTR(" edge \r\n")
#endif
#endif

/* unknown reason to reset init_start */
reset_init_start:
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

wait_init_done:
    dli     t1, 0x160
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000ff000000
    and     a0, a0, t4
    beqz    a0, wait_init_done
    nop

    dli     t3, 0x1
    b       glvl_req_set
    nop

#if 1
dll_gate_0_add0:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 0 add\r\n")
#endif
    dli     t1, 0x038
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu   a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x028
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x030
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 0 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_1_set0
    dli     s7, 0x1 
dll_gate_1_add0:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 1 add\r\n")
#endif
    dli     t1, 0x058
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x048
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x050
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 1 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_2_set0
    dli     s7, 0x1
dll_gate_2_add0:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 2 add\r\n")
#endif
    dli     t1, 0x078
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x068
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x070
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 2 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_3_set0
    dli     s7, 0x1
dll_gate_3_add0:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 3 add\r\n")
#endif
    dli     t1, 0x098
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x088
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x090
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 3 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_4_set0
    dli     s7, 0x1
dll_gate_4_add0:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 4 add\r\n")
#endif
    dli     t1, 0x0b8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//	  //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x0a8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x0b0
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 4 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_5_set0
    dli     s7, 0x1
dll_gate_5_add0:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 5 add\r\n")
#endif
    dli     t1, 0x0d8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x0c8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x0d0
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 5 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_6_set0
    dli     s7, 0x1
dll_gate_6_add0:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 6 add\r\n")
#endif
    dli     t1, 0x0f8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x0e8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x0f0
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 6 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_7_set0
    dli     s7, 0x1
dll_gate_7_add0:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 7 add\r\n")
#endif
    dli     t1, 0x118
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x108
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x110
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 7 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_8_set0
    dli     s7, 0x1
dll_gate_8_add0:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 8 add\r\n")
#endif
    dli     t1, 0x138
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x128
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x130
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 0, slice 8 oe and odt add\r\n")
#endif
1:
    b       glvl_req_set
    dli     s7, 0x1
#endif

glvl_resp_set:
//	PRINTSTR("\r\n All set to 0")
//  dli     s6, 0x1
glvl_resp_0_set1:
    dli     s7, 0x0
    dli     t1, 0x180
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0300000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 56
    beq     a0, $0, dll_gate_0_add
    nop
glvl_resp_1_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000000003
    and     a0, a0, t4
    beq     a0, $0, dll_gate_1_add
    nop
glvl_resp_2_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000000300
    and     a0, a0, t4
    dsrl    a0, a0, 8
    beq     a0, $0, dll_gate_2_add
    nop
glvl_resp_3_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000000030000
    and     a0, a0, t4
    dsrl    a0, a0, 16
    beq     a0, $0, dll_gate_3_add
    nop
glvl_resp_4_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000003000000
    and     a0, a0, t4
    dsrl    a0, a0, 24
    beq     a0, $0, dll_gate_4_add
    nop
glvl_resp_5_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000000300000000
    and     a0, a0, t4
    dsrl    a0, a0, 32
    beq     a0, $0, dll_gate_5_add
    nop
glvl_resp_6_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0000030000000000
    and     a0, a0, t4
    dsrl    a0, a0, 40
    beq     a0, $0, dll_gate_6_add
    nop
glvl_resp_7_set1:
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0003000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 48
    beq     a0, $0, dll_gate_7_add
    nop
glvl_resp_8_set1:
/* identify wheather there is ecc slice */
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    dli     t1, 0x188
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x0300000000000000
    and     a0, a0, t4
    dsrl    a0, a0, 56
    beq     a0, $0, dll_gate_8_add
    nop

1:
    beqz    s7, 1f 
    nop
	dli		s6, 0x6
	b		glvl_req_set
	nop
1:
#ifdef FILTER_CHECK
glvl_check_5_slice0:
	bnez	t0, glvl_check_5_slice1
	nop
//	PRINTSTR("\r\nglvl_check slice 0\r\n")
//	move	a0, t0
//	bal		hexserial
//	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x38
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	b		glvl_req_set
	nop
1:
	PRINTSTR("\r\nt0 add 1\r\n")
    dli     t1, 0x38
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	blt		a0, 0x7b, 1f
	nop
	dsubu	t1, t1, 0x10
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
	daddu	t1, t1, 0x10
1:
	dli		s6, 0x6
	daddu	t0, t0, 0x1
glvl_check_5_slice1:
	bne		t0, 1, glvl_check_5_slice2
	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x58
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	b		glvl_req_set
	nop
1:
    dli     t1, 0x58
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	blt		a0, 0x7b, 1f
	nop
	dsubu	t1, t1, 0x10
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
	daddu	t1, t1, 0x10
1:
	dli		s6, 0x6
	daddu	t0, t0, 0x1
glvl_check_5_slice2:
	bne		t0, 2, glvl_check_5_slice3
	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x78
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	b		glvl_req_set
	nop
1:
    dli     t1, 0x78
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	blt		a0, 0x7b, 1f
	nop
	dsubu	t1, t1, 0x10
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
	daddu	t1, t1, 0x10
1:
	dli		s6, 0x6
	daddu	t0, t0, 0x1
glvl_check_5_slice3:
	bne		t0, 3, glvl_check_5_slice4
	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x98
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	b		glvl_req_set
	nop
1:
    dli     t1, 0x98
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	blt		a0, 0x7b, 1f
	nop
	dsubu	t1, t1, 0x10
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
	daddu	t1, t1, 0x10
1:
	dli		s6, 0x6
	daddu	t0, t0, 0x1
glvl_check_5_slice4:
	bne		t0, 4, glvl_check_5_slice5
	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0xb8
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	b		glvl_req_set
	nop
1:
    dli     t1, 0xb8
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	blt		a0, 0x7b, 1f
	nop
	dsubu	t1, t1, 0x10
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
	daddu	t1, t1, 0x10
1:
	dli		s6, 0x6
	daddu	t0, t0, 0x1
glvl_check_5_slice5:
	bne		t0, 5, glvl_check_5_slice6
	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0xd8
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	b		glvl_req_set
	nop
1:
    dli     t1, 0xd8
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	blt		a0, 0x7b, 1f
	nop
	dsubu	t1, t1, 0x10
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
	daddu	t1, t1, 0x10
1:
	dli		s6, 0x6
	daddu	t0, t0, 0x1
glvl_check_5_slice6:
	bne		t0, 6, glvl_check_5_slice7
	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0xf8
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	b		glvl_req_set
	nop
1:
    dli     t1, 0xf8
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	blt		a0, 0x7b, 1f
	nop
	dsubu	t1, t1, 0x10
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
	daddu	t1, t1, 0x10
1:
	dli		s6, 0x6
	daddu	t0, t0, 0x1
glvl_check_5_slice7:
	bne		t0, 7, glvl_check_5_slice8
	nop
//	PRINTSTR("\r\nglvl_check slice 7\r\n")
//	move	a0, t0
//	bal		hexserial
//	nop
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x118
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	b		glvl_req_set
	nop
1:
    dli     t1, 0x118
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	blt		a0, 0x7b, 1f
	nop
	dsubu	t1, t1, 0x10
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
	daddu	t1, t1, 0x10
1:
	dli		s6, 0x6
	daddu	t0, t0, 0x1
glvl_check_5_slice8:
/* identify wheather there is ecc slice */
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, glvl_check_5_end //when rd_after_write enabled, the 9th device may don't need leveling
    nop
1:
	subu	s6, s6, 0x1
	beqz	s6, 1f
	nop
	
    dli     t1, 0x138
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    daddu   a0, a0, 0x1
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	b		glvl_req_set
	nop
1:
    dli     t1, 0x138
    or      t1, t1, t8
    lb      a0, 0x0(t1)
    dsubu   a0, a0, 0x5
    dli     t4, 0x7f
    and     a0, a0, t4
    sb      a0, 0x0(t1)
	blt		a0, 0x7b, 1f
	nop
	dsubu	t1, t1, 0x10
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
	daddu	t1, t1, 0x10
1:
	dli		s6, 0x0
	daddu	t0, t0, 0x1
glvl_check_5_end:
PRINTSTR("\r\nfiltering glvl_check success\r\n")
#endif


#ifdef  PRINT_DDR_LEVELING   //print registers
    PRINTSTR("\r\nThe MC param after gate leveling 0 to 1 is:\r\n")
    dli     t1, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, 0x900000000ff00000
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
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
	
    daddiu  t1, t1, -1
    daddiu  t5, t5, 8
    bnez    t1, 1b
	nop

#ifdef EDGES_CHECK_DEBUG	
	PRINTSTR("\r\nThe above is NO.")
	move	a0, s6
	bal		hexserial
	nop
	PRINTSTR(" edge \r\n")
#endif

#endif

#ifdef	PREMBLE_CHECK_DEBUG
#ifdef	EDGES_CHECK_DEBUG	
	bnez	s6, 111f
	nop
#endif

	dli		s7, 0x8
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
	daddu	s7, s7, 0x1
1:
	dli		t1, 0x28
	or 		t1, t1, t8
	dli		t2, 0x180
	or		t2, t2, t8
premble_check_init:
/* check the premble exist */
	PRINTSTR("\r\nPREMBLE CHECK!!\r\n")
// set the gate signal 0.75 period before
	dli 	s6,	0x60 //s6 represents 0.75 period to be checked	
	dli		t4, 0x0
	or		t4, t4, t8
	lb		a0, 0x0(t4)
	beq		a0, 0x2, 1f
	nop
	dli		s6, 0x5a
1:
#ifdef DDR_DLL_BYPASS
	lbu		t4, 0x4(t8)
	daddu	t4, t4, 0x2
	move	a0, t4
	dsrl	t4, t4, 0x2
	dsubu	a0, a0, t4
	move	s6, a0
#endif


	lb		a0, 0x7(t1)
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
	dsubu	a0, a0, s6
	dli		t4, 0x7f
	and		a0, a0, t4
#ifdef DDR_DLL_BYPASS
	dli		t4, 0x80
	dsubu	t4, t4, s6
	blt		a0, t4, 11f
	nop

	lbu		t4, 0x4(t8)
	daddu	t4, t4, 0x2
	daddu	a0, a0, t4
	sb		a0, 0x10(t1)

	lb		a0, 0x7(t1)
	dsubu	a0, a0, 1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 1
	sb 		a0, 0x6(t1)
	lb		a0, 0x7(t1)
	RDOE_ADD_TRDDATA_SUB
	b		22f
	nop
11:
	ori		a0, 0x80
	sb		a0, 0x10(t1)
22:
#else
	sb		a0, 0x10(t1)
	dli		t4, 0x80
	dsubu	t4, t4, s6
	blt		a0, t4, 1f
	nop
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 1 //dq_oe_end=0?
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 1
	sb 		a0, 0x6(t1)
	lb		a0, 0x7(t1)
	RDOE_ADD_TRDDATA_SUB
#endif

1:
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
	dsubu	s6, s6, 0x6
	b 		glvl_redo_req_set_0
	nop
glvl_check_premble:


	dsubu	s6, s6, 0x1
	bnez	s6, 1f
	nop
	daddu	s6, s6, 0x1
1:

    lb      a0, 0x7(t2)
    dli     t4, 0x3
    and     a0, a0, t4
	
	bnez	a0, test_continuous5_0
	nop

	//PRINTSTR("The 1 is not found\r\n")

	lb		a0, 0x10(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and 	a0, a0, t4
	sb		a0, 0x10(t1)
#ifdef DDR_DLL_BYPASS
	lbu		t4, 0x4(t8)
	daddu	t4, t4, 0x2
#if 0
	move	a1, a0
	bal		hexserial
	nop
	move	a0,	a1
#endif
	bltu	a0, t4, 11f
	nop
	dli		a0, 0x80
	sb		a0, 0x10(t1)
	dli		a0, 0x0
	b		22f
	nop
11:
	ori		a0, 0x80
	sb		a0, 0x10(t1)
22:
#endif

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
#ifdef DDR_DLL_BYPASS
	dli		t6, 0x0
#endif
	bnez	t6, 1f
	nop
	beq		s6, 0x1, glvl_check_premble_end
	nop
	b		glvl_check_premble_fail
	nop
1:
#ifdef PRINT_PREMBLE_CHECK 
	PRINTSTR("The 1 found in premble test@")
	move	a0, s6
	bal 	hexserial
	nop
	move	a0, t6
	bal 	hexserial
	nop
	PRINTSTR("\r\n")
#endif

	lb		a0, 0x10(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x10(t1)
#ifdef DDR_DLL_BYPASS
	lbu		t4, 0x4(t8)
	daddu	t4, t4, 0x2
	blt		a0, t4, 11f
	nop
	dli		a0, 0x80
	sb		a0, 0x10(t1)
	dli		a0, 0x0
	b		22f
	nop
11:
	ori		a0, 0x80
	sb		a0, 0x10(t1)
22:
#endif
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

glvl_check_premble_fail:
	PRINTSTR("\r\nThe premble check failed @")
	move	a0, s6
	bal 	hexserial
	nop
	PRINTSTR("\r\n")

	dli		s6, 0x0
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	bnez	a0, 1f
	nop
	PRINTSTR("\r\nThe rd_oe become 0 in the premble check!\r\n")
	RDOE_ADD_TRDDATA_SUB
1:

	
	dli		t3, 0x0
glvl_redo_req_set_0:
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

	beq		t3, 0x1, glvl_redo_resp_set1_0	
	nop

	beq		t3, 0x2, glvl_check_premble
	nop

#ifdef	FIND_PREMBLE_LENGTH
	beq		t3, 0x3, premble_check_length
	nop
#endif

	dli		t3, 0x1
    lb      a0, 0x7(t2)
    dli     t4, 0x3
    and     a0, a0, t4
    beq     a0, 0x0, 1f
    nop
	
	lb		a0, 0x10(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x10(t1)
#ifdef DDR_DLL_BYPASS
	lbu		t4, 0x4(t8)
	daddu	t4, t4, 0x2
	blt		a0, t4, 11f
	nop
	dli		a0, 0x80
	sb		a0, 0x10(t1)
	dli		a0, 0x0
	b		22f
	nop
11:
	ori		a0, 0x80
	sb		a0, 0x10(t1)
22:
#endif
	dli		t3, 0x0
	bnez	a0, 1f
	nop

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

glvl_redo_resp_set1_0:
	
    lb      a0, 0x7(t2)
    dli     t4, 0x3
    and     a0, a0, t4
    bnez    a0, premble_check_init
    nop

	lb		a0, 0x10(t1)
	daddu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x10(t1)
#ifdef DDR_DLL_BYPASS
	lbu		t4, 0x4(t8)
	daddu	t4, t4, 0x2
	blt		a0, t4, 11f
	nop
	dli		a0, 0x80
	sb		a0, 0x10(t1)
	dli		a0, 0x0
	b		22f
	nop
11:
	ori		a0, 0x80
	sb		a0, 0x10(t1)
22:
#endif

	bnez	a0, 1f
	nop

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


glvl_check_premble_end:
#ifdef  PRINT_PREMBLE_CHECK   //print registers
    PRINTSTR("\r\nThe MC param after premble check is:\r\n")
    dli     t4, DDR_PARAM_NUM
    GET_NODE_ID_a0
    dli     t5, 0x900000000ff00000
    or      t5, t5, a0
1:
    ld      t6, 0x0(t5)
    move    a0, t5
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
	
    daddiu  t4, t4, -1
    daddiu  t5, t5, 8
    bnez    t4, 1b
	nop

#ifdef EDGES_CHECK_DEBUG	
	PRINTSTR("\r\nThe above is NO.")
	move	a0, s6
	bal		hexserial
	nop
	PRINTSTR(" edge \r\n")
#endif
#endif
	dli		s6, 0x0
	PRINTSTR("\r\nThe premble check success\r\n")
	
	lb		a0, 0x7(t1)
	blt		a0, 0x4, 1f
	nop
	dsubu	a0, a0, 0x4
	sb		a0, 0x7(t1)
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
1:
#ifdef DDR_DLL_BYPASS
#else
	lb		a0, 0x10(t1)
	dsubu	a0, a0, 0x4
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x10(t1)
	ble		a0, 0x7b, 1f
	nop
	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
1:
#endif
//set rddqs_lt_half
	lb		a0, 0x10(t1)
	dli		t4, 0x7f
	and		a0, a0, t4
#ifdef DDR_DLL_BYPASS
    dsll    a0, a0, 0x7 // x 128
    lw 	    t5, 0x4(t8) //get dll_ck value, store at t5
    daddu   t5, t5, 0x2
    divu    a0, a0, t5 //get dll_gate, no bypass mode
#endif
 	lb		a1, 0x11(t1)
	daddu	a0, a0, a1
	and		a0, a0, t4
#if 1
	move	a1, a0
	bal		hexserial
	nop
	move	a0, a1
#endif
	bgeu	a0, 0x40, rddqs_lthalf_set0
	nop
	bltu	a0,	0x0, rddqs_lthalf_set0
	nop
	b		rddqs_lthalf_set1
	nop
rddqs_lthalf_set0:
	dsubu	t4, t1, 0x8
	dli		a0, 0x0
	sb		a0, 0x2(t4)
	b		1f
	nop
rddqs_lthalf_set1:
	dsubu	t4, t1, 0x8
	dli		a0, 0x1
	sb		a0, 0x2(t4)
1:
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

dll_gate_sub_20:
#ifdef DDR_DLL_BYPASS
	lb		a0, 0x10(t1)
	bal		hexserial
	nop
	lbu		t4, 0x4(t8)
	daddu	t4, t4, 0x2
	dsrl	t4, t4, 0x2
	move	a1, t4
	lbu		a0, 0x10(t1)
	dli		t4, 0x7f
	and		a0, a0, t4
	blt		a0, a1, 11f
	nop
	dsubu	a0, a0, a1
	ori		a0, 0x80
	sb		a0, 0x10(t1)
	b		22f
	nop
11:
	dli		a0, 0x80
	sb		a0, 0x10(t1)
22:
#else
	lb		a0, 0x10(t1)
	dsubu	a0, a0, 0x20
	dli		t4, 0x7f
	and		a0, a0, t4
	sb		a0, 0x10(t1)
	blt		a0, 0x60, 1f
	nop

	lb		a0, 0x7(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t1)
	lb		a0, 0x6(t1)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t1)
	RDOE_ADD_TRDDATA_SUB
1:
#endif

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

get_burst_length: //save in t9
#ifdef DDR_DLL_BYPASS
#else
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
	b		premble_check_init
	nop
#endif
glvl_last_check_end:
	PRINTSTR("\r\nThe edges number is correct!\r\n")
	daddu	t1, t1, 0x20
	daddu	t2, t2, 0x1
	dsubu	s7, s7, 0x1
	bnez	s7, premble_check_init
	nop
111:
#ifdef FIND_PREMBLE_LENGTH
	dli		t3, 0x3
	dli		t4, 0x38
	or		t4, t4, t8
	lb		a0, 0x0(t4)
	dsubu	a0, a0, 0x5
	dli		t4, 0x7f
	and		a0, a0, t4
	dli		t4, 0x38
	or		t4, t4, t8
	sb		a0, 0x0(t4)
	blt		a0,	0x7b, 1f
	nop
	dli		t4, 0x28
	or		t4, t4,t8
	lb		a0, 0x7(t4)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t4)
	lb		a0, 0x6(t4)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t4)
	RDOE_ADD_TRDDATA_SUB
	dli		s6, 0x0
	PRINTSTR("111111")
1:
	b		glvl_redo_req_set_0
	nop
premble_check_length:
    dli		t4, 0x180
	or		t4, t4, t8
	lb      a0, 0x7(t4)
    dli     t4, 0x3
    and     a0, a0, t4
    beq     a0, 0x1, 2f
    nop
	
	PRINTSTR("22222")

	daddu	s6, s6, 0x1
	bge		s6, 0xff, 2f
	dli		t4, 0x38
	or		t4, t4, t8
	lb		a0, 0x0(t4)
	dsubu	a0, a0, 0x1
	dli		t4, 0x7f
	and		a0, a0, t4
	dli		t4, 0x38
	or		t4, t4, t8
	sb		a0, 0x0(t4)
	blt		a0,	0x7f, 1f
	nop
	dli		t4, 0x28
	or		t4, t4,t8
	lb		a0, 0x7(t4)
	dsubu	a0, a0, 0x1
	sb		a0, 0x7(t4)
	lb		a0, 0x6(t4)
	dsubu	a0, a0, 0x1
	sb		a0, 0x6(t4)
	RDOE_ADD_TRDDATA_SUB
1:
	b		glvl_redo_req_set_0
	nop
2:
	PRINTSTR("\r\n the premle length is")
	move	a0, s6
	bal		hexserial
	nop

#endif
	b		gate_leveling_exit               //-------------------------the after code is not effect-until gate leveling exit-------------------------------------------
	nop										//--------------------------------------!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!-------------------------------------------------------

#endif

#ifdef EDGES_CHECK_DEBUG
	beq		s6, 0x5, posedges_check_end
	nop
/* check 4 posedges exist */
	daddu 	s6, s6, 0x1
	dli		t3, 0x0
	bne		s6, 0x5, glvl_req_set
	nop

//identify wheather there is ecc slice
	li      t0, 0x8
	dli     t1, 0x250
	or      t1, t1, t8
	lb      a0, 0x2(t1)
	dli     t1, 0x1
	and     a0, a0, t1
	bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
	nop
	daddu   t0, t0, 0x1
1:
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
	
//identify wheather there is ecc slice
	li      t0, 0x8
	dli     t1, 0x250
	or      t1, t1, t8
	lb      a0, 0x2(t1)
	dli     t1, 0x1
	and     a0, a0, t1
	bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
	nop
	daddu   t0, t0, 0x1
1:
	dli		t1, 0x30
	or		t1, t1, t8
reset_odt_oe:
	dli		a0, 0x2
	sb		a0, 0x2(t1)
	sb		a0, 0x3(t1)
	daddu	t1, t1, 0x20
	dsubu	t0, t0, 0x1
	bnez	t0, reset_odt_oe
	nop

//identify wheather there is ecc slice
	li      t0, 0x8
	dli     t1, 0x250
	or      t1, t1, t8
	lb      a0, 0x2(t1)
	dli     t1, 0x1
	and     a0, a0, t1
	bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
	nop
	daddu   t0, t0, 0x1
1:
    dli     t1, 0x018
    or      t1, t1, t8
dll_gate_set0_2:
    daddu    t1, t1, 0x20
    ld      a0, 0x0(t1)
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
#ifdef DDR_DLL_BYPASS
    dli     t4, 0x0000000000000080
    or      a0, a0, t4
#endif
    sd      a0, 0x0(t1)
    subu    t0, t0, 0x1 
    bnez    t0, dll_gate_set0_2 
    nop

	b		glvl_req_set
	nop 
posedges_check_end:
#endif

#if 1
#if 1
/* unknown reason to reset init_start */
reset_init_start0:
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

wait_init_done0:
    dli     t1, 0x160
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000ff000000
    and     a0, a0, t4
    beqz    a0, wait_init_done0
    nop
#endif

/* identify wheather there is ecc slice */
    li      t0, 0x8
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
    dli     a2, 0x1a
    dli     a3, 0x3a
    dli     t1, 0x018
    or      t1, t1, t8
rddqs_lt_half:
    beq     t0, $0, dll_gate_set
    nop
    subu    t0, t0, 0x1
    daddu   t1, t1, 0x20
    ld      a0, 0x0(t1)
    move    a1, a0
    dli     t4, 0x000000000000007f
    dli     t6, 0x7f //dll value limit
    and     a0, a0, t4 //get dll_gate, store at a0, remove high bit 1
#ifdef DDR_DLL_BYPASS
    dsll    a0, a0, 0x7 // x 128
    lw      t5, 0x4(t8) //get dll_ck value, store at t5
    daddu   t5, t5, 0x2
    divu    a0, a0, t5 //get dll_gate, no bypass mode
#endif
    dli     t5, 0x000000000000ff00
    and     a1, a1, t5
    dsrl    a1, a1, 8  //get dll_wrdata
    daddu   a0, a0, a1
    and     a0, a0, t6
#if 0
	move	a1, a0
	bal		hexserial
	nop
	move	a0, a1
#endif
    bgeu    a0, a3, rddqs_lt_half_set1//because the rd gate edge is 0x2
    nop
    bltu    a0, a2, rddqs_lt_half_set1
    nop
    b       rddqs_lt_half_set0
    nop
rddqs_lt_half_set0:
    dsubu   t2, t1, 0x18
    ld      a0, 0x0(t2)
    dli     t4, 0xffffffffff00ffff
    and     a0, a0, t4
    sd      a0, 0x0(t2)
    b       rddqs_lt_half
    nop
rddqs_lt_half_set1:
    dsubu   t2, t1, 0x18
    ld      a0, 0x0(t2)
    dli     t4, 0xffffffffff00ffff
    and     a0, a0, t4
    dli     t4, 0x10000
    or      a0, a0, t4
    sd      a0, 0x0(t2)
    b       rddqs_lt_half 
    nop

#if 1
/* unknown reason to reset init_start */
reset_init_start1:
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

wait_init_done1:
    dli     t1, 0x160
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000ff000000
    and     a0, a0, t4
    beqz    a0, wait_init_done1
    nop
#endif

dll_gate_set:
/* identify wheather there is ecc slice */
    li      t0, 0x8
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1
1:
#ifdef DDR_DLL_BYPASS
    lw      a2, 0x4(t8) //dll_value_ck
    daddu   a2, a2, 0x2
    dsrl    a2, a2, 0x2
    ori     a2, a2, 0x80 //set high bit
    dli     t4, 0x00000000000000ff
    and     a2, a2, t4
#else
    dli     a2, 0x0a
#endif
    dli     t1, 0x018
    or      t1, t1, t8
dll_gate_set_loop:
    beq     t0, $0, rd_oe_sub
    //beq     t0, $0, gate_leveling_exit
    nop
    subu    t0, t0, 0x1
    daddu    t1, t1, 0x20
    ld      a0, 0x0(t1)
    move    a1, a0
    dli     t4, 0x00000000000000ff
    and     a1, a1, t4
    bgtu    a1, a2, dll_gate_sub20
    nop
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
#ifdef DDR_DLL_BYPASS
    ori     a0, a0, 0x80
#endif
//    daddu   a0, a0, 0x60
    sd      a0, 0x0(t1)

///* sub rd_oe_begin-end */
//    dli     t4, 0x10
//    dsubu   t1, t1, t4
//    ld      a0, 0x0(t1)
//    dli     t4, 0x0101000000000000
//    dsubu   a0, a0, t4
//    sd      a0, 0x0(t1)
//    daddu   t1, t1, 0x10
//
///* sub odt_oe_begin-end */
//    dli     t4, 0x8
//    dsubu   t1, t1, t4
//    ld      a0, 0x0(t1)
//    dli     t4, 0x0000000001010000
//    dsubu   a0, a0, t4
////    sd      a0, 0x0(t1)
//    daddu   t1, t1, 0x8

    b       dll_gate_set_loop
    nop
dll_gate_sub20:
    dsubu    a1, a1, a2
#ifdef DDR_DLL_BYPASS
    ori     a1, a1, 0x80
#endif
    dli     t4, 0xffffffffffffff00
    and     a0, a0, t4
    daddu    a0, a0, a1
    sd      a0, 0x0(t1)
    b       dll_gate_set_loop
    nop

#if 1
/* unknown reason to reset init_start */
reset_init_start2:
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

wait_init_done2:
    dli     t1, 0x160
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t4, 0x00000000ff000000
    and     a0, a0, t4
    beqz    a0, wait_init_done2
    nop
#endif
#endif

dll_gate_0_add:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 0 add\r\n")
#endif
    dli     t1, 0x038
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x028
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x030
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 0 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_1_set1
    dli     s7, 0x1 
dll_gate_1_add:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 1 add\r\n")
#endif
    dli     t1, 0x058
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x048
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x050
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 1 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_2_set1
    dli     s7, 0x1
dll_gate_2_add:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 2 add\r\n")
#endif
    dli     t1, 0x078
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
   nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x068
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x070
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 2 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_3_set1
    dli     s7, 0x1
dll_gate_3_add:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 3 add\r\n")
#endif
    dli     t1, 0x098
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x088
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x090
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 3 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_4_set1
    dli     s7, 0x1
dll_gate_4_add:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 4 add\r\n")
#endif
    dli     t1, 0x0b8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x0a8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x0b0
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 4 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_5_set1
    dli     s7, 0x1
dll_gate_5_add:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 5 add\r\n")
#endif
    dli     t1, 0x0d8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x0c8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x0d0
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 5 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_6_set1
    dli     s7, 0x1
dll_gate_6_add:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 6 add\r\n")
#endif
    dli     t1, 0x0f8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x0e8
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x0f0
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 6 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_7_set1
    dli     s7, 0x1
dll_gate_7_add:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 7 add\r\n")
#endif
    dli     t1, 0x118
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x108
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x110
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 7 oe and odt add\r\n")
#endif
1:
    b       glvl_resp_8_set1
    dli     s7, 0x1
dll_gate_8_add:
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 8 add\r\n")
#endif
    dli     t1, 0x138
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    daddu    a0, a0, 0x1
#ifdef DDR_DLL_BYPASS
    lw      t2, 0x4(t8) //get dll_value
    daddu   t2, t2, 0x2
    dli     t4, 0x7f
    and     a0, a0, t4
    blt     a0, t2, 2f //use blt because the dll_value may change
    nop
    dli     a0, 0x80
    sb      a0, 0x0(t1)
    b       3f
    nop

2:  
    ori     a0, 0x80
    sb      a0, 0x0(t1)
    b       1f
    nop
    
#else
    dli     t2, 0xffffffffffffff7f
    and     a0, a0, t2
    sd      a0, 0x0(t1)
    dli     t4, 0x7f
    and     a0, a0, t4
    bnez    a0, 1f
    nop
//    //set dll_gate to 0
//    ld      a0, 0x0(t1)
//    dli     t2, 0xffffffffffffff00
//    and     a0, a0, t2
//    sd      a0, 0x0(t1)
#endif
    
3:
    
    /* rd_oe_begin and rd_oe_end add 1 */
    dli     t1, 0x128
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0101000000000000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
	lb		a0, 0x7(t1)
	RDOE_SUB_TRDDATA_ADD
    /* odt_oe_begin and odt_oe_end add 1 */
    dli     t1, 0x130
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    daddu   a0, a0, t2
    sd      a0, 0x0(t1)
#ifdef LVL_DEBUG
    PRINTSTR("\r\nfor all gate resp got a 1, slice 8 oe and odt add\r\n")
#endif
1:
    b       glvl_req_set
    dli     s7, 0x1

rd_oe_sub:

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
/*
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
*/

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
   //sd      a0, 0x0(t1)

    dli     t1, 0x0f0
    or      t1, t1, t8
    ld      a0, 0x0(t1)
    dli     t2, 0x0000000001010000
    dsubu   a0, a0, t2
   //sd      a0, 0x0(t1)
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

gate_leveling_exit:
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
#endif

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
    li      t0, 0x8
    dli     t1, 0x250
    or      t1, t1, t8
    lb      a0, 0x2(t1)
    dli     t1, 0x1
    and     a0, a0, t1
    bne     a0, t1, 1f //when rd_after_write enabled, the 9th device may don't need leveling
    nop
    daddu   t0, t0, 0x1

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
 
