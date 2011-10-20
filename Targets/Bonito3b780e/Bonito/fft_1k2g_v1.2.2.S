#function:1k point fft
##author: L.L
##version: 1k2g_v1.0
##date: 2009/09/21
##revised: 2009/09/21 by L.L 
#include "regdef.h"
##include "mipsregs.h"
##include "cacheops.h"
##include "newinst.h"
##include "main.h"

//#define fft_dinm       a0
//#define Dout_addr     a1
//#define Dout_base1    a2
//#define DMACFG_BASE   a3

#define DMACFG_BASE  0x900000001fa00000
#define A_STRIDE     0x80
#define B_STRIDE     0x08
#define C_OFFSET     0xe0
#define D_STRIDE     0x200
#define Dout_base1    0x9800000000120000
//#define Dout_base2    0x9800000000130000

#define bc2f(cc, offset) \
        .word ((0x248 << 21) | (cc << 18) | (0 << 16) | (offset))

        .text
        .set    noreorder
        .set    mips64

        .align 5
        .global fft_1kp
        .ent    fft_1kp
fft_1kp:
        daddi	sp, sp, -256                      
        sd 	s0, 0x00(sp)
    	sd	s1, 0x08(sp)
    	sd	s2, 0x10(sp)
    	sd	s3, 0x18(sp)
    	sd	s4, 0x20(sp)
    	sd	s5, 0x28(sp)
    	sd	s6, 0x30(sp)
    	sd	s7, 0x38(sp)
        sd	ra, 0x40(sp)                         #  *sp=ra($31 reg,)

#if 0
       dla     a0, fft_1kp_begin                  #  a0=label_addr
       dla     t9, outputstring
       jalr    t9
#endif

       nop
       dli  a0,0x01000000
       ctc1 zero,ra
        
       dli t3, 0x8000000000100000           ##lock address:1fc00000~1fc0ffff
       dli t4, 0x900000003ff00000
       sd  t3, 0x218(t4) 

       dli t4, 0xfffffffffffc0000           ##mask     
       dli t4, 0x900000003ff00000
       sd  t3, 0x258(t4)

fft_function:

init: 
        move s0,$0
        addu s0,s0,0x10
        move s1,$0
        move s2,$0
        move s3,$0
        move s4,$0
        addu s4,s4,0x01
        move s7,$0
        vzero $z1      #din_A
        vzero $z2
        vzero $z3
        vzero $z4              
        vzero $z5      #din_B
        vzero $z6
        vzero $z7
        vzero $z8
        vzero $z9      #din_W
        vzero $z10
        vzero $z11
        vzero $z12
        
        vzero $z21      #result
        vzero $z22
        vzero $z23
        vzero $z24              
        vzero $z25      #
        vzero $z26
        vzero $z27
        vzero $z28

        vzero $z31      #din_A dma_des 
        vzero $z32
        vzero $z33
        vzero $z34              
        vzero $z35      #din_B dma_des
        vzero $z36
        vzero $z37
        vzero $z38

##        SET_PERFCNT0(PC_CYCLE)       

first_dma_conf:
##axi to vec0 for din_A         
        dli  t4, 0x07780080010200          ##times=0x10=16,length=0x200=512bytes,m_stride=0x200=512, arcache=0xf, arcmd=0xe
        dli  t6, 0x0204007f001000       ##cycle_r_addr=0x7f=31, r_addr=0x00, r_stride=0x1, seg_pin_count=1, seg_step=0x200
##        dli  t0, DMACFG_BASE
        move t0, DMACFG_BASE
        dla  t1, w_din_dat
        move t2, Dout_base1  ## for dma out
        dla  t3, fft_din
##        dla  t5, Dout_base2  ## for dma out
        daddu t5, t2, 0x2000 
        dli  t7, 0x1
##        sq  (_t6, _t4, _t0, 1)       
##        sq  (_t7, _t7, _t0, 8)
        sd  t4, 0x10(t0)
        sd  t6, 0x18(t0)       
        sd  t7, 0x80(t0)       
        sd  t7, 0x88(t0)       

##conf Cr24
##        dli  t6, 0x1f010301020101  ##or ad, dma_end_en=0b11111=0x1f
        dli  t6, 0x1f407130410140  ##or ad, dma_end_en=0b11111=0x1f
 ##conf Cr27 and Cr28
        dli  t4, 0xffffffffe0000000
        sd   t6, 0xc0(t0)
        sd   t4, 0xd8(t0)
        dli  t6, 0x0
        sd   t6, 0xe0(t0)

start_din_dma_1:
##start axi to vec0
        sd   t3, 0x0(t0)
        daddu   t4, t1, 0x80
##move twiddle factor to reg.
        vldql $z58,0(t1)
        vldql $z52,0x10(t1)
        vldql $z59,0x20(t1)
        vldql $z53,0x30(t1)
        vldql $z60,0x40(t1)
        vldql $z54,0x50(t1)
        vldql $z61,0x60(t1)
        vldql $z55,0x70(t1)        
        vldql $z62,0x0(t4)
        vldql $z56,0x10(t4)
        vldql $z63,0x20(t4)
        vldql $z57,0x30(t4)

##axi to vec0 for din_C        
        dli  t4, 0x07780080010180          ##times=0x10=16,length=0x180=384bytes,m_stride=0x200=512, arcache=0xf, arcmd=0xe
        dli  t6, 0x000300ff0010d0          ##cycle_r_addr=0xff=63, r_addr=0xd0=52, r_stride=0x1, seg_pin_count=0, seg_step=0x180

        vpermutqi   $z58, $z58, $z52, 0x20
        vpermutqi   $z59, $z59, $z53, 0x20
        vpermutqi   $z60, $z60, $z54, 0x20
        vpermutqi   $z61, $z61, $z55, 0x20
        vpermutqi   $z62, $z62, $z56, 0x20
        vpermutqi   $z63, $z63, $z57, 0x20

##        sq (_t6, _t4, _t0, 5)
        sd  t4, 0x50(t0)
        sd  t6, 0x58(t0)       
        nop
conf_dout_dma_1: ##Cr8~Cr9
##vec to axi
        dli  t4, 0x06f80080010200     ##times=0x10=16,length=0x200=512bytes,m_stride=0x200=512, arcache=0xf, arcmd=0xd, send_ar=0
        dli  t6, 0x18004007c004000       ##cycle_r_addr=0x7c=31, r_addr=0x0, r_stride=0x4, seg_pin_count=0, seg_step=0x200, fft_clr=1, fft_step=0, wall=1

##clear Cr22 dma_end[4]
        sd   t7, 0xb0(t0)
##        sq (_t6, _t4, _t0, 4)     
        sd  t4, 0x40(t0)
        sd  t6, 0x48(t0)       

##first group
1:      bc2f(0, 0xffff)
        nop
fft_stage_1_g1:       
##A->A
        vcpfftps.lpre $z32, $z8 , $z58, $z0 ##1 
        vcpfftps.lpre $z36, $z12, $z58, $z4 ##9
        vcpfftps.lpre $z33, $z9 , $z58, $z1 ##3       
        vcpfftps.lpre $z37, $z13, $z58, $z5 ##11       
        vcpfftps.lpre $z34, $z10, $z58, $z2 ##5
        vcpfftps.lpre $z38, $z14, $z58, $z6 ##13
        vcpfftps.lpre $z35, $z11, $z58, $z3 ##7       
        vcpfftps.lpre $z39, $z15, $z58, $z7 ##15       
                                    
        vcpfftps.hpre $z40, $z8 , $z58, $z0 ##2
        vcpfftps.hpre $z44, $z12, $z58, $z4 ##10
        vcpfftps.hpre $z41, $z9 , $z58, $z1 ##4      
        vcpfftps.hpre $z45, $z13, $z58, $z5 ##12
        vcpfftps.hpre $z42, $z10, $z58, $z2 ##6     
        vcpfftps.hpre $z46, $z14, $z58, $z6 ##14
        vcpfftps.hpre $z43, $z11, $z58, $z3 ##8
        vcpfftps.hpre $z47, $z15, $z58, $z7 ##16

        vcpfftps.l $z0 , $z8 , $z58, $z32
        vcpfftps.l $z4 , $z12, $z58, $z36
        vcpfftps.l $z2 , $z10, $z58, $z34
        vcpfftps.l $z6 , $z14, $z58, $z38
        vcpfftps.l $z1 , $z9 , $z58, $z33
        vcpfftps.l $z5 , $z13, $z58, $z37
        vcpfftps.l $z3 , $z11, $z58, $z35
        vcpfftps.l $z7 , $z15, $z58, $z39
                                         
        vcpfftps.h $z8 , $z8 , $z58, $z40
        vcpfftps.h $z12, $z12, $z58, $z44
        vcpfftps.h $z9 , $z9 , $z58, $z41
        vcpfftps.h $z13, $z13, $z58, $z45
        vcpfftps.h $z10, $z10, $z58, $z42
        vcpfftps.h $z14, $z14, $z58, $z46
        vcpfftps.h $z11, $z11, $z58, $z43
        vcpfftps.h $z15, $z15, $z58, $z47
                                           
fft_stage_2_g1:       
##A->C
        vcpfftps.lpre $z32, $z4 , $z59, $z0   
        vcpfftps.hpre $z36, $z4 , $z59, $z0  
        vcpfftps.lpre $z34, $z6 , $z59, $z2   
        vcpfftps.hpre $z38, $z6 , $z59, $z2     
        vcpfftps.lpre $z33, $z5 , $z59, $z1        
        vcpfftps.hpre $z37, $z5 , $z59, $z1      
        vcpfftps.lpre $z35, $z7 , $z59, $z3       
        vcpfftps.hpre $z39, $z7 , $z59, $z3  

        vcpfftps.lpre $z40, $z12, $z59, $z8 
        vcpfftps.hpre $z44, $z12, $z59, $z8  
        vcpfftps.lpre $z42, $z14, $z59, $z10 
        vcpfftps.hpre $z46, $z14, $z59, $z10 
        vcpfftps.lpre $z41, $z13, $z59, $z9       
        vcpfftps.hpre $z45, $z13, $z59, $z9  
        vcpfftps.lpre $z43, $z15, $z59, $z11       
        vcpfftps.hpre $z47, $z15, $z59, $z11 

        vcpfftps.l $z32, $z4 , $z59, $z32 
        vcpfftps.l $z34, $z6 , $z59, $z34 
        vcpfftps.h $z36, $z4 , $z59, $z36 
        vcpfftps.h $z38, $z6 , $z59, $z38 
        vcpfftps.l $z33, $z5 , $z59, $z33         
        vcpfftps.l $z35, $z7 , $z59, $z35        
        vcpfftps.h $z37, $z5 , $z59, $z37  
        vcpfftps.h $z39, $z7 , $z59, $z39 

        vcpfftps.l $z40, $z12, $z59, $z40 
        vcpfftps.l $z42, $z14, $z59, $z42
        vcpfftps.l $z41, $z13, $z59, $z41         
        vcpfftps.l $z43, $z15, $z59, $z43        
        vcpfftps.h $z44, $z12, $z59, $z44 
        vcpfftps.h $z46, $z14, $z59, $z46 
        vcpfftps.h $z45, $z13, $z59, $z45 
        vcpfftps.h $z47, $z15, $z59, $z47 
                                             
fft_stage_3_g1:       
##C->A
        vcpfftps.lpre $z0 , $z34, $z60, $z32  
        vcpfftps.hpre $z2 , $z34, $z60, $z32  
        vcpfftps.lpre $z1 , $z35, $z60, $z33      
        vcpfftps.hpre $z3 , $z35, $z60, $z33     
        vcpfftps.lpre $z4 , $z38, $z60, $z36  
        vcpfftps.hpre $z6 , $z38, $z60, $z36     
        vcpfftps.lpre $z5 , $z39, $z60, $z37       
        vcpfftps.hpre $z7 , $z39, $z60, $z37  
        
        vcpfftps.lpre $z8 , $z42, $z60, $z40  
        vcpfftps.lpre $z9 , $z43, $z60, $z41        
        vcpfftps.hpre $z10, $z42, $z60, $z40 
        vcpfftps.hpre $z11, $z43, $z60, $z41 
        vcpfftps.lpre $z12, $z46, $z60, $z44  
        vcpfftps.hpre $z14, $z46, $z60, $z44  
        vcpfftps.lpre $z13, $z47, $z60, $z45        
        vcpfftps.hpre $z15, $z47, $z60, $z45  

        vcpfftps.l $z0 , $z34, $z60, $z0  
        vcpfftps.l $z1 , $z35, $z60, $z1         
        vcpfftps.h $z2 , $z34, $z60, $z2  
        vcpfftps.h $z3 , $z35, $z60, $z3  
        vcpfftps.l $z4 , $z38, $z60, $z4  
        vcpfftps.l $z5 , $z39, $z60, $z5        
        vcpfftps.l $z8 , $z42, $z60, $z8  
        vcpfftps.l $z9 , $z43, $z60, $z9       

        vcpfftps.h $z6 , $z38, $z60, $z6  
        vcpfftps.h $z7 , $z39, $z60, $z7  
        vcpfftps.h $z10, $z42, $z60, $z10 
        vcpfftps.h $z11, $z43, $z60, $z11 
        vcpfftps.l $z12, $z46, $z60, $z12 
        vcpfftps.l $z13, $z47, $z60, $z13       
        vcpfftps.h $z14, $z46, $z60, $z14 
        vcpfftps.h $z15, $z47, $z60, $z15 
                                             
fft_stage_4_g1:       
##A->C
        vcpfftps.lpre $z32, $z1 , $z60, $z0  
        vcpfftps.hpre $z33, $z1 , $z60, $z0 
        vcpfftps.lpre $z34, $z3 , $z61, $z2        
        vcpfftps.hpre $z35, $z3 , $z61, $z2      
        vcpfftps.lpre $z40, $z9 , $z60, $z8 
        vcpfftps.hpre $z41, $z9 , $z60, $z8  
        vcpfftps.lpre $z36, $z5 , $z60, $z4  
        vcpfftps.hpre $z37, $z5 , $z60, $z4    

        vcpfftps.lpre $z42, $z11, $z61, $z10       
        vcpfftps.hpre $z43, $z11, $z61, $z10
        vcpfftps.lpre $z38, $z7 , $z61, $z6        
        vcpfftps.hpre $z39, $z7 , $z61, $z6 
        vcpfftps.lpre $z44, $z13, $z60, $z12 
        vcpfftps.hpre $z45, $z13, $z60, $z12 
        vcpfftps.lpre $z46, $z15, $z61, $z14       
        vcpfftps.hpre $z47, $z15, $z61, $z14 

        vcpfftps.l $z32, $z1 , $z60, $z32 
        vcpfftps.h $z33, $z1 , $z60, $z33 
        vcpfftps.l $z40, $z9 , $z60, $z40
        vcpfftps.h $z41, $z9 , $z60, $z41
        vcpfftps.l $z34, $z3 , $z61, $z34        
        vcpfftps.h $z35, $z3 , $z61, $z35 
        vcpfftps.l $z42, $z11, $z61, $z42      
        vcpfftps.h $z43, $z11, $z61, $z43 
        
        vcpfftps.l $z36, $z5 , $z60, $z36 
        vcpfftps.h $z37, $z5 , $z60, $z37 
        vcpfftps.l $z44, $z13, $z60, $z44 
        vcpfftps.h $z45, $z13, $z60, $z45
        vcpfftps.l $z38, $z7 , $z61, $z38        
        vcpfftps.h $z39, $z7 , $z61, $z39 
        vcpfftps.l $z46, $z15, $z61, $z46       
        vcpfftps.h $z47, $z15, $z61, $z47 
                                             
fft_stage_5_g1:        
##C->A
        vcpfftps.lpre $z0 , $z40, $z60, $z32
        vcpfftps.lpre $z4 , $z41, $z61, $z33        
        vcpfftps.hpre $z8 , $z40, $z60, $z32
        vcpfftps.hpre $z12, $z41, $z61, $z33      
        vcpfftps.lpre $z2 , $z42, $z62, $z34
        vcpfftps.lpre $z6 , $z43, $z63, $z35       
        vcpfftps.hpre $z10, $z42, $z62, $z34
        vcpfftps.hpre $z14, $z43, $z63, $z35

        vcpfftps.lpre $z1 , $z44, $z60, $z36
        vcpfftps.lpre $z5 , $z45, $z61, $z37        
        vcpfftps.hpre $z9 , $z44, $z60, $z36
        vcpfftps.hpre $z13, $z45, $z61, $z37      
        vcpfftps.lpre $z3 , $z46, $z62, $z38
        vcpfftps.lpre $z7 , $z47, $z63, $z39        
        vcpfftps.hpre $z11, $z46, $z62, $z38
        vcpfftps.hpre $z15, $z47, $z63, $z39
       
        vcpfftps.l $z0 , $z40, $z60, $z0 
        vcpfftps.l $z4 , $z41, $z61, $z4 
        vcpfftps.h $z8 , $z40, $z60, $z8 
        vcpfftps.h $z12, $z41, $z61, $z12
        vcpfftps.l $z2 , $z42, $z62, $z2 
        vcpfftps.l $z6 , $z43, $z63, $z6 
        vcpfftps.h $z10, $z42, $z62, $z10
        vcpfftps.h $z14, $z43, $z63, $z14

        vcpfftps.l $z1 , $z44, $z60, $z1         
        vcpfftps.l $z5 , $z45, $z61, $z5       
        vcpfftps.h $z9 , $z44, $z60, $z9         
        vcpfftps.h $z13, $z45, $z61, $z13
        vcpfftps.l $z3 , $z46, $z62, $z3         
        vcpfftps.l $z7 , $z47, $z63, $z7        
        vcpfftps.h $z11, $z46, $z62, $z11        
        vcpfftps.h $z15, $z47, $z63, $z15
                                          
        sd   t2, 0x30(t0)
        nop

##second group
1:      bc2f(1, 0xffff)
        nop
fft_stage_1_g2:       
##A->A
        vcpfftps.lpre $z32, $z24, $z58, $z16##1 
        vcpfftps.lpre $z36, $z28, $z58, $z20##9
        vcpfftps.lpre $z33, $z25, $z58, $z17##3       
        vcpfftps.lpre $z37, $z29, $z58, $z21##11       
        vcpfftps.lpre $z34, $z26, $z58, $z18##5
        vcpfftps.lpre $z38, $z30, $z58, $z22##13
        vcpfftps.lpre $z35, $z27, $z58, $z19##7       
        vcpfftps.lpre $z39, $z31, $z58, $z23##15       
                                    
        vcpfftps.hpre $z40, $z24, $z58, $z16##2
        vcpfftps.hpre $z44, $z28, $z58, $z20##10
        vcpfftps.hpre $z41, $z25, $z58, $z17##4      
        vcpfftps.hpre $z45, $z29, $z58, $z21##12
        vcpfftps.hpre $z42, $z26, $z58, $z18##6     
        vcpfftps.hpre $z46, $z30, $z58, $z22##14
        vcpfftps.hpre $z43, $z27, $z58, $z19##8
        vcpfftps.hpre $z47, $z31, $z58, $z23##16

        vcpfftps.l $z16, $z24, $z58, $z32
        vcpfftps.l $z20, $z28, $z58, $z36
        vcpfftps.l $z18, $z26, $z58, $z34
        vcpfftps.l $z22, $z30, $z58, $z38
        vcpfftps.l $z17, $z25, $z58, $z33
        vcpfftps.l $z21, $z29, $z58, $z37
        vcpfftps.l $z19, $z27, $z58, $z35
        vcpfftps.l $z23, $z31, $z58, $z39
                                         
        vcpfftps.h $z24, $z24, $z58, $z40
        vcpfftps.h $z28, $z28, $z58, $z44
        vcpfftps.h $z25, $z25, $z58, $z41
        vcpfftps.h $z29, $z29, $z58, $z45
        vcpfftps.h $z26, $z26, $z58, $z42
        vcpfftps.h $z30, $z30, $z58, $z46
        vcpfftps.h $z27, $z27, $z58, $z43
        vcpfftps.h $z31, $z31, $z58, $z47
                                           
fft_stage_2_g2:       
##A->C
        vcpfftps.lpre $z32, $z20, $z59, $z16
        vcpfftps.hpre $z36, $z20, $z59, $z16 
        vcpfftps.lpre $z34, $z22, $z59, $z18
        vcpfftps.hpre $z38, $z22, $z59, $z18    
        vcpfftps.lpre $z33, $z21, $z59, $z17
        vcpfftps.hpre $z37, $z21, $z59, $z17     
        vcpfftps.lpre $z35, $z23, $z59, $z19
        vcpfftps.hpre $z39, $z23, $z59, $z19 

        vcpfftps.lpre $z40, $z28, $z59, $z24
        vcpfftps.hpre $z44, $z28, $z59, $z24
        vcpfftps.lpre $z42, $z30, $z59, $z26
        vcpfftps.hpre $z46, $z30, $z59, $z26 
        vcpfftps.lpre $z41, $z29, $z59, $z25
        vcpfftps.hpre $z45, $z29, $z59, $z25 
        vcpfftps.lpre $z43, $z31, $z59, $z27
        vcpfftps.hpre $z47, $z31, $z59, $z27 

        bc2f(6, 0xffff)
        nop
        sd  t7, 0x80(t0)
        nop
        vcpfftps.l $z32, $z20, $z59, $z32 
        vcpfftps.l $z34, $z22, $z59, $z34 
        vcpfftps.h $z36, $z20, $z59, $z36 
        vcpfftps.h $z38, $z22, $z59, $z38 
        vcpfftps.l $z33, $z21, $z59, $z33         
        vcpfftps.l $z35, $z23, $z59, $z35        
        vcpfftps.h $z37, $z21, $z59, $z37  
        vcpfftps.h $z39, $z23, $z59, $z39 
        
        vcpfftps.l $z40, $z28, $z59, $z40 
        vcpfftps.l $z42, $z30, $z59, $z42 
        vcpfftps.l $z41, $z29, $z59, $z41         
        vcpfftps.l $z43, $z31, $z59, $z43        
        vcpfftps.h $z44, $z28, $z59, $z44 
        vcpfftps.h $z46, $z30, $z59, $z46 
        vcpfftps.h $z45, $z29, $z59, $z45 
        vcpfftps.h $z47, $z31, $z59, $z47 
                                             
fft_stage_3_g2:       
##C->A
        vcpfftps.lpre $z16, $z34, $z60, $z32  
        vcpfftps.hpre $z18, $z34, $z60, $z32  
        vcpfftps.lpre $z17, $z35, $z60, $z33       
        vcpfftps.hpre $z19, $z35, $z60, $z33      
        vcpfftps.lpre $z20, $z38, $z60, $z36  
        vcpfftps.hpre $z22, $z38, $z60, $z36     
        vcpfftps.lpre $z21, $z39, $z60, $z37      
        vcpfftps.hpre $z23, $z39, $z60, $z37  

        vcpfftps.lpre $z24, $z42, $z60, $z40  
        vcpfftps.lpre $z25, $z43, $z60, $z41        
        vcpfftps.hpre $z26, $z42, $z60, $z40  
        vcpfftps.hpre $z27, $z43, $z60, $z41  
        vcpfftps.lpre $z28, $z46, $z60, $z44  
        vcpfftps.hpre $z30, $z46, $z60, $z44  
        vcpfftps.lpre $z29, $z47, $z60, $z45        
        vcpfftps.hpre $z31, $z47, $z60, $z45  

        vcpfftps.l $z16, $z34, $z60, $z16
        vcpfftps.l $z17, $z35, $z60, $z17      
        vcpfftps.h $z18, $z34, $z60, $z18
        vcpfftps.h $z19, $z35, $z60, $z19
        vcpfftps.l $z20, $z38, $z60, $z20
        vcpfftps.l $z21, $z39, $z60, $z21      
        vcpfftps.l $z24, $z42, $z60, $z24
        vcpfftps.l $z25, $z43, $z60, $z25     

        vcpfftps.h $z22, $z38, $z60, $z22
        vcpfftps.h $z23, $z39, $z60, $z23
        vcpfftps.h $z26, $z42, $z60, $z26
        vcpfftps.h $z27, $z43, $z60, $z27
        vcpfftps.l $z28, $z46, $z60, $z28
        vcpfftps.l $z29, $z47, $z60, $z29     
        vcpfftps.h $z30, $z46, $z60, $z30
        vcpfftps.h $z31, $z47, $z60, $z31
                                             
fft_stage_4_g2:       
##A->C
        vcpfftps.lpre $z32, $z17, $z60, $z16
        vcpfftps.hpre $z33, $z17, $z60, $z16
        vcpfftps.lpre $z34, $z19, $z61, $z18
        vcpfftps.hpre $z35, $z19, $z61, $z18
        vcpfftps.lpre $z40, $z25, $z60, $z24
        vcpfftps.hpre $z41, $z25, $z60, $z24
        vcpfftps.lpre $z36, $z21, $z60, $z20
        vcpfftps.hpre $z37, $z21, $z60, $z20

        vcpfftps.lpre $z42, $z27, $z61, $z26
        vcpfftps.hpre $z43, $z27, $z61, $z26
        vcpfftps.lpre $z38, $z23, $z61, $z22
        vcpfftps.hpre $z39, $z23, $z61, $z22
        vcpfftps.lpre $z44, $z29, $z60, $z28
        vcpfftps.hpre $z45, $z29, $z60, $z28
        vcpfftps.lpre $z46, $z31, $z61, $z30
        vcpfftps.hpre $z47, $z31, $z61, $z30

        vcpfftps.l $z32, $z17, $z60, $z32 
        vcpfftps.h $z33, $z17, $z60, $z33 
        vcpfftps.l $z40, $z25, $z60, $z40 
        vcpfftps.h $z41, $z25, $z60, $z41 
        vcpfftps.l $z34, $z19, $z61, $z34        
        vcpfftps.h $z35, $z19, $z61, $z35
        vcpfftps.l $z42, $z27, $z61, $z42        
        vcpfftps.h $z43, $z27, $z61, $z43 

        vcpfftps.l $z36, $z21, $z60, $z36 
        vcpfftps.h $z37, $z21, $z60, $z37 
        vcpfftps.l $z44, $z29, $z60, $z44 
        vcpfftps.h $z45, $z29, $z60, $z45 
        vcpfftps.l $z38, $z23, $z61, $z38        
        vcpfftps.h $z39, $z23, $z61, $z39 
        vcpfftps.l $z46, $z31, $z61, $z46       
        vcpfftps.h $z47, $z31, $z61, $z47 
                                             
fft_stage_5_g2:        
##C->A
        vcpfftps.lpre $z16, $z40, $z60, $z32
        vcpfftps.hpre $z24, $z40, $z60, $z32
        vcpfftps.lpre $z20, $z41, $z61, $z33        
        vcpfftps.hpre $z28, $z41, $z61, $z33      
        vcpfftps.lpre $z18, $z42, $z62, $z34
        vcpfftps.hpre $z26, $z42, $z62, $z34
        vcpfftps.lpre $z22, $z43, $z63, $z35        
        vcpfftps.hpre $z30, $z43, $z63, $z35
        
        vcpfftps.lpre $z17, $z44, $z60, $z36
        vcpfftps.hpre $z25, $z44, $z60, $z36
        vcpfftps.lpre $z21, $z45, $z61, $z37        
        vcpfftps.hpre $z29, $z45, $z61, $z37       
        vcpfftps.lpre $z19, $z46, $z62, $z38
        vcpfftps.hpre $z27, $z46, $z62, $z38
        vcpfftps.lpre $z23, $z47, $z63, $z39        
        vcpfftps.hpre $z31, $z47, $z63, $z39
       
        vcpfftps.l $z16, $z40, $z60, $z16
        vcpfftps.l $z20, $z41, $z61, $z20
        vcpfftps.h $z24, $z40, $z60, $z24
        vcpfftps.h $z28, $z41, $z61, $z28
        vcpfftps.l $z18, $z42, $z62, $z18
        vcpfftps.l $z22, $z43, $z63, $z22
        vcpfftps.h $z26, $z42, $z62, $z26
        vcpfftps.h $z30, $z43, $z63, $z30

        vcpfftps.l $z17, $z44, $z60, $z17       
        vcpfftps.l $z21, $z45, $z61, $z21     
        vcpfftps.h $z25, $z44, $z60, $z25       
        vcpfftps.h $z29, $z45, $z61, $z29
        vcpfftps.l $z19, $z46, $z62, $z19       
        vcpfftps.l $z23, $z47, $z63, $z23      
        vcpfftps.h $z27, $z46, $z62, $z27       
        vcpfftps.h $z31, $z47, $z63, $z31

        sd   t7, 0xb0(t0)
        li   s0, 0x6
fft_cal_1:
##third group
1:      bc2f(0, 0xffff)
        nop
fft_stage_1_g3:       
##A->A
        vcpfftps.lpre $z32, $z8 , $z58, $z0 ##1 
        vcpfftps.lpre $z36, $z12, $z58, $z4 ##9
        vcpfftps.lpre $z33, $z9 , $z58, $z1 ##3       
        vcpfftps.lpre $z37, $z13, $z58, $z5 ##11       
        vcpfftps.lpre $z34, $z10, $z58, $z2 ##5
        vcpfftps.lpre $z38, $z14, $z58, $z6 ##13
        vcpfftps.lpre $z35, $z11, $z58, $z3 ##7       
        vcpfftps.lpre $z39, $z15, $z58, $z7 ##15       
                                    
        vcpfftps.hpre $z40, $z8 , $z58, $z0 ##2
        vcpfftps.hpre $z44, $z12, $z58, $z4 ##10
        vcpfftps.hpre $z41, $z9 , $z58, $z1 ##4      
        vcpfftps.hpre $z45, $z13, $z58, $z5 ##12
        vcpfftps.hpre $z42, $z10, $z58, $z2 ##6     
        vcpfftps.hpre $z46, $z14, $z58, $z6 ##14
        vcpfftps.hpre $z43, $z11, $z58, $z3 ##8
        vcpfftps.hpre $z47, $z15, $z58, $z7 ##16

        vcpfftps.l $z0 , $z8 , $z58, $z32
        vcpfftps.l $z4 , $z12, $z58, $z36
        vcpfftps.l $z2 , $z10, $z58, $z34
        vcpfftps.l $z6 , $z14, $z58, $z38
        vcpfftps.l $z1 , $z9 , $z58, $z33
        vcpfftps.l $z5 , $z13, $z58, $z37
        vcpfftps.l $z3 , $z11, $z58, $z35
        vcpfftps.l $z7 , $z15, $z58, $z39
                                         
        vcpfftps.h $z8 , $z8 , $z58, $z40
        vcpfftps.h $z12, $z12, $z58, $z44
        vcpfftps.h $z9 , $z9 , $z58, $z41
        vcpfftps.h $z13, $z13, $z58, $z45
        vcpfftps.h $z10, $z10, $z58, $z42
        vcpfftps.h $z14, $z14, $z58, $z46
        vcpfftps.h $z11, $z11, $z58, $z43
        vcpfftps.h $z15, $z15, $z58, $z47
                                           
fft_stage_2_g3:       
##A->C
        vcpfftps.lpre $z32, $z4 , $z59, $z0   
        vcpfftps.hpre $z36, $z4 , $z59, $z0  
        vcpfftps.lpre $z34, $z6 , $z59, $z2   
        vcpfftps.hpre $z38, $z6 , $z59, $z2     
        vcpfftps.lpre $z33, $z5 , $z59, $z1        
        vcpfftps.hpre $z37, $z5 , $z59, $z1      
        vcpfftps.lpre $z35, $z7 , $z59, $z3       
        vcpfftps.hpre $z39, $z7 , $z59, $z3  

        vcpfftps.lpre $z40, $z12, $z59, $z8 
        vcpfftps.hpre $z44, $z12, $z59, $z8  
        vcpfftps.lpre $z42, $z14, $z59, $z10 
        vcpfftps.hpre $z46, $z14, $z59, $z10 
        vcpfftps.lpre $z41, $z13, $z59, $z9       
        vcpfftps.hpre $z45, $z13, $z59, $z9  
        vcpfftps.lpre $z43, $z15, $z59, $z11       
        vcpfftps.hpre $z47, $z15, $z59, $z11 

        bc2f(6, 0xffff)
        nop
        sd  t7, 0x88(t0)
        nop
             
        vcpfftps.l $z32, $z4 , $z59, $z32 
        vcpfftps.l $z34, $z6 , $z59, $z34 
        vcpfftps.h $z36, $z4 , $z59, $z36 
        vcpfftps.h $z38, $z6 , $z59, $z38 
        vcpfftps.l $z33, $z5 , $z59, $z33         
        vcpfftps.l $z35, $z7 , $z59, $z35        
        vcpfftps.h $z37, $z5 , $z59, $z37  
        vcpfftps.h $z39, $z7 , $z59, $z39 

        vcpfftps.l $z40, $z12, $z59, $z40 
        vcpfftps.l $z42, $z14, $z59, $z42
        vcpfftps.l $z41, $z13, $z59, $z41         
        vcpfftps.l $z43, $z15, $z59, $z43        
        vcpfftps.h $z44, $z12, $z59, $z44 
        vcpfftps.h $z46, $z14, $z59, $z46 
        vcpfftps.h $z45, $z13, $z59, $z45 
        vcpfftps.h $z47, $z15, $z59, $z47 
             
fft_stage_3_g3:       
##C->A
        vcpfftps.lpre $z0 , $z34, $z60, $z32  
        vcpfftps.hpre $z2 , $z34, $z60, $z32  
        vcpfftps.lpre $z1 , $z35, $z60, $z33      
        vcpfftps.hpre $z3 , $z35, $z60, $z33     
        vcpfftps.lpre $z4 , $z38, $z60, $z36  
        vcpfftps.hpre $z6 , $z38, $z60, $z36     
        vcpfftps.lpre $z5 , $z39, $z60, $z37       
        vcpfftps.hpre $z7 , $z39, $z60, $z37  
        
        vcpfftps.lpre $z8 , $z42, $z60, $z40  
        vcpfftps.lpre $z9 , $z43, $z60, $z41        
        vcpfftps.hpre $z10, $z42, $z60, $z40 
        vcpfftps.hpre $z11, $z43, $z60, $z41 
        vcpfftps.lpre $z12, $z46, $z60, $z44  
        vcpfftps.hpre $z14, $z46, $z60, $z44  
        vcpfftps.lpre $z13, $z47, $z60, $z45        
        vcpfftps.hpre $z15, $z47, $z60, $z45  

        vcpfftps.l $z0 , $z34, $z60, $z0  
        vcpfftps.l $z1 , $z35, $z60, $z1         
        vcpfftps.h $z2 , $z34, $z60, $z2  
        vcpfftps.h $z3 , $z35, $z60, $z3  
        vcpfftps.l $z4 , $z38, $z60, $z4  
        vcpfftps.l $z5 , $z39, $z60, $z5        
        vcpfftps.l $z8 , $z42, $z60, $z8  
        vcpfftps.l $z9 , $z43, $z60, $z9       

        vcpfftps.h $z6 , $z38, $z60, $z6  
        vcpfftps.h $z7 , $z39, $z60, $z7  
        vcpfftps.h $z10, $z42, $z60, $z10 
        vcpfftps.h $z11, $z43, $z60, $z11 
        vcpfftps.l $z12, $z46, $z60, $z12 
        vcpfftps.l $z13, $z47, $z60, $z13       
        vcpfftps.h $z14, $z46, $z60, $z14 
        vcpfftps.h $z15, $z47, $z60, $z15 
                                             
fft_stage_4_g3:       
##A->C
        vcpfftps.lpre $z32, $z1 , $z60, $z0  
        vcpfftps.hpre $z33, $z1 , $z60, $z0 
        vcpfftps.lpre $z34, $z3 , $z61, $z2        
        vcpfftps.hpre $z35, $z3 , $z61, $z2      
        vcpfftps.lpre $z40, $z9 , $z60, $z8 
        vcpfftps.hpre $z41, $z9 , $z60, $z8  
        vcpfftps.lpre $z36, $z5 , $z60, $z4  
        vcpfftps.hpre $z37, $z5 , $z60, $z4    

        vcpfftps.lpre $z42, $z11, $z61, $z10       
        vcpfftps.hpre $z43, $z11, $z61, $z10
        vcpfftps.lpre $z38, $z7 , $z61, $z6        
        vcpfftps.hpre $z39, $z7 , $z61, $z6 
        vcpfftps.lpre $z44, $z13, $z60, $z12 
        vcpfftps.hpre $z45, $z13, $z60, $z12 
        vcpfftps.lpre $z46, $z15, $z61, $z14       
        vcpfftps.hpre $z47, $z15, $z61, $z14 

        vcpfftps.l $z32, $z1 , $z60, $z32 
        vcpfftps.h $z33, $z1 , $z60, $z33 
        vcpfftps.l $z40, $z9 , $z60, $z40
        vcpfftps.h $z41, $z9 , $z60, $z41
        vcpfftps.l $z34, $z3 , $z61, $z34        
        vcpfftps.h $z35, $z3 , $z61, $z35 
        vcpfftps.l $z42, $z11, $z61, $z42      
        vcpfftps.h $z43, $z11, $z61, $z43 
        
        vcpfftps.l $z36, $z5 , $z60, $z36 
        vcpfftps.h $z37, $z5 , $z60, $z37 
        vcpfftps.l $z44, $z13, $z60, $z44 
        vcpfftps.h $z45, $z13, $z60, $z45
        vcpfftps.l $z38, $z7 , $z61, $z38        
        vcpfftps.h $z39, $z7 , $z61, $z39 
        vcpfftps.l $z46, $z15, $z61, $z46       
        vcpfftps.h $z47, $z15, $z61, $z47 
                                             
fft_stage_5_g3:        
##C->A
        vcpfftps.lpre $z0 , $z40, $z60, $z32
        vcpfftps.lpre $z4 , $z41, $z61, $z33        
        vcpfftps.hpre $z8 , $z40, $z60, $z32
        vcpfftps.hpre $z12, $z41, $z61, $z33      
        vcpfftps.lpre $z2 , $z42, $z62, $z34
        vcpfftps.lpre $z6 , $z43, $z63, $z35       
        vcpfftps.hpre $z10, $z42, $z62, $z34
        vcpfftps.hpre $z14, $z43, $z63, $z35

        vcpfftps.lpre $z1 , $z44, $z60, $z36
        vcpfftps.lpre $z5 , $z45, $z61, $z37        
        vcpfftps.hpre $z9 , $z44, $z60, $z36
        vcpfftps.hpre $z13, $z45, $z61, $z37      
        vcpfftps.lpre $z3 , $z46, $z62, $z38
        vcpfftps.lpre $z7 , $z47, $z63, $z39        
        vcpfftps.hpre $z11, $z46, $z62, $z38
        vcpfftps.hpre $z15, $z47, $z63, $z39
       
        vcpfftps.l $z0 , $z40, $z60, $z0 
        vcpfftps.l $z4 , $z41, $z61, $z4 
        vcpfftps.h $z8 , $z40, $z60, $z8 
        vcpfftps.h $z12, $z41, $z61, $z12
        vcpfftps.l $z2 , $z42, $z62, $z2 
        vcpfftps.l $z6 , $z43, $z63, $z6 
        vcpfftps.h $z10, $z42, $z62, $z10
        vcpfftps.h $z14, $z43, $z63, $z14

        vcpfftps.l $z1 , $z44, $z60, $z1         
        vcpfftps.l $z5 , $z45, $z61, $z5       
        vcpfftps.h $z9 , $z44, $z60, $z9         
        vcpfftps.h $z13, $z45, $z61, $z13
        vcpfftps.l $z3 , $z46, $z62, $z3         
        vcpfftps.l $z7 , $z47, $z63, $z7        
        vcpfftps.h $z11, $z46, $z62, $z11        
        vcpfftps.h $z15, $z47, $z63, $z15
                                          
        sd   t7, 0xb0(t0)
        nop

##fourth group
1:      bc2f(1, 0xffff)
        nop
fft_stage_1_g4:       
##A->A
        vcpfftps.lpre $z32, $z24, $z58, $z16##1 
        vcpfftps.lpre $z36, $z28, $z58, $z20##9
        vcpfftps.lpre $z33, $z25, $z58, $z17##3       
        vcpfftps.lpre $z37, $z29, $z58, $z21##11       
        vcpfftps.lpre $z34, $z26, $z58, $z18##5
        vcpfftps.lpre $z38, $z30, $z58, $z22##13
        vcpfftps.lpre $z35, $z27, $z58, $z19##7       
        vcpfftps.lpre $z39, $z31, $z58, $z23##15       
                                    
        vcpfftps.hpre $z40, $z24, $z58, $z16##2
        vcpfftps.hpre $z44, $z28, $z58, $z20##10
        vcpfftps.hpre $z41, $z25, $z58, $z17##4      
        vcpfftps.hpre $z45, $z29, $z58, $z21##12
        vcpfftps.hpre $z42, $z26, $z58, $z18##6     
        vcpfftps.hpre $z46, $z30, $z58, $z22##14
        vcpfftps.hpre $z43, $z27, $z58, $z19##8
        vcpfftps.hpre $z47, $z31, $z58, $z23##16

        vcpfftps.l $z16, $z24, $z58, $z32
        vcpfftps.l $z20, $z28, $z58, $z36
        vcpfftps.l $z18, $z26, $z58, $z34
        vcpfftps.l $z22, $z30, $z58, $z38
        vcpfftps.l $z17, $z25, $z58, $z33
        vcpfftps.l $z21, $z29, $z58, $z37
        vcpfftps.l $z19, $z27, $z58, $z35
        vcpfftps.l $z23, $z31, $z58, $z39
                                         
        vcpfftps.h $z24, $z24, $z58, $z40
        vcpfftps.h $z28, $z28, $z58, $z44
        vcpfftps.h $z25, $z25, $z58, $z41
        vcpfftps.h $z29, $z29, $z58, $z45
        vcpfftps.h $z26, $z26, $z58, $z42
        vcpfftps.h $z30, $z30, $z58, $z46
        vcpfftps.h $z27, $z27, $z58, $z43
        vcpfftps.h $z31, $z31, $z58, $z47
                                           
fft_stage_2_g4:       
##A->C
        vcpfftps.lpre $z32, $z20, $z59, $z16
        vcpfftps.hpre $z36, $z20, $z59, $z16 
        vcpfftps.lpre $z34, $z22, $z59, $z18
        vcpfftps.hpre $z38, $z22, $z59, $z18    
        vcpfftps.lpre $z33, $z21, $z59, $z17
        vcpfftps.hpre $z37, $z21, $z59, $z17     
        vcpfftps.lpre $z35, $z23, $z59, $z19
        vcpfftps.hpre $z39, $z23, $z59, $z19 

        vcpfftps.lpre $z40, $z28, $z59, $z24
        vcpfftps.hpre $z44, $z28, $z59, $z24
        vcpfftps.lpre $z42, $z30, $z59, $z26
        vcpfftps.hpre $z46, $z30, $z59, $z26 
        vcpfftps.lpre $z41, $z29, $z59, $z25
        vcpfftps.hpre $z45, $z29, $z59, $z25 
        vcpfftps.lpre $z43, $z31, $z59, $z27
        vcpfftps.hpre $z47, $z31, $z59, $z27 

        bc2f(6, 0xffff)
        nop
        sd  t7, 0x80(t0)
        nop
        vcpfftps.l $z32, $z20, $z59, $z32 
        vcpfftps.l $z34, $z22, $z59, $z34 
        vcpfftps.h $z36, $z20, $z59, $z36 
        vcpfftps.h $z38, $z22, $z59, $z38 
        vcpfftps.l $z33, $z21, $z59, $z33         
        vcpfftps.l $z35, $z23, $z59, $z35        
        vcpfftps.h $z37, $z21, $z59, $z37  
        vcpfftps.h $z39, $z23, $z59, $z39 
        
        vcpfftps.l $z40, $z28, $z59, $z40 
        vcpfftps.l $z42, $z30, $z59, $z42 
        vcpfftps.l $z41, $z29, $z59, $z41         
        vcpfftps.l $z43, $z31, $z59, $z43        
        vcpfftps.h $z44, $z28, $z59, $z44 
        vcpfftps.h $z46, $z30, $z59, $z46 
        vcpfftps.h $z45, $z29, $z59, $z45 
        vcpfftps.h $z47, $z31, $z59, $z47 
                                             
fft_stage_3_g4:       
##C->A
        vcpfftps.lpre $z16, $z34, $z60, $z32  
        vcpfftps.hpre $z18, $z34, $z60, $z32  
        vcpfftps.lpre $z17, $z35, $z60, $z33       
        vcpfftps.hpre $z19, $z35, $z60, $z33      
        vcpfftps.lpre $z20, $z38, $z60, $z36  
        vcpfftps.hpre $z22, $z38, $z60, $z36     
        vcpfftps.lpre $z21, $z39, $z60, $z37      
        vcpfftps.hpre $z23, $z39, $z60, $z37  

        vcpfftps.lpre $z24, $z42, $z60, $z40  
        vcpfftps.lpre $z25, $z43, $z60, $z41        
        vcpfftps.hpre $z26, $z42, $z60, $z40  
        vcpfftps.hpre $z27, $z43, $z60, $z41  
        vcpfftps.lpre $z28, $z46, $z60, $z44  
        vcpfftps.hpre $z30, $z46, $z60, $z44  
        vcpfftps.lpre $z29, $z47, $z60, $z45        
        vcpfftps.hpre $z31, $z47, $z60, $z45  

        vcpfftps.l $z16, $z34, $z60, $z16
        vcpfftps.l $z17, $z35, $z60, $z17      
        vcpfftps.h $z18, $z34, $z60, $z18
        vcpfftps.h $z19, $z35, $z60, $z19
        vcpfftps.l $z20, $z38, $z60, $z20
        vcpfftps.l $z21, $z39, $z60, $z21      
        vcpfftps.l $z24, $z42, $z60, $z24
        vcpfftps.l $z25, $z43, $z60, $z25     

        vcpfftps.h $z22, $z38, $z60, $z22
        vcpfftps.h $z23, $z39, $z60, $z23
        vcpfftps.h $z26, $z42, $z60, $z26
        vcpfftps.h $z27, $z43, $z60, $z27
        vcpfftps.l $z28, $z46, $z60, $z28
        vcpfftps.l $z29, $z47, $z60, $z29     
        vcpfftps.h $z30, $z46, $z60, $z30
        vcpfftps.h $z31, $z47, $z60, $z31
                                             
fft_stage_4_g4:       
##A->C
        vcpfftps.lpre $z32, $z17, $z60, $z16
        vcpfftps.hpre $z33, $z17, $z60, $z16
        vcpfftps.lpre $z34, $z19, $z61, $z18
        vcpfftps.hpre $z35, $z19, $z61, $z18
        vcpfftps.lpre $z40, $z25, $z60, $z24
        vcpfftps.hpre $z41, $z25, $z60, $z24
        vcpfftps.lpre $z36, $z21, $z60, $z20
        vcpfftps.hpre $z37, $z21, $z60, $z20

        vcpfftps.lpre $z42, $z27, $z61, $z26
        vcpfftps.hpre $z43, $z27, $z61, $z26
        vcpfftps.lpre $z38, $z23, $z61, $z22
        vcpfftps.hpre $z39, $z23, $z61, $z22
        vcpfftps.lpre $z44, $z29, $z60, $z28
        vcpfftps.hpre $z45, $z29, $z60, $z28
        vcpfftps.lpre $z46, $z31, $z61, $z30
        vcpfftps.hpre $z47, $z31, $z61, $z30

        vcpfftps.l $z32, $z17, $z60, $z32 
        vcpfftps.h $z33, $z17, $z60, $z33 
        vcpfftps.l $z40, $z25, $z60, $z40 
        vcpfftps.h $z41, $z25, $z60, $z41 
        vcpfftps.l $z34, $z19, $z61, $z34        
        vcpfftps.h $z35, $z19, $z61, $z35
        vcpfftps.l $z42, $z27, $z61, $z42        
        vcpfftps.h $z43, $z27, $z61, $z43 

        vcpfftps.l $z36, $z21, $z60, $z36 
        vcpfftps.h $z37, $z21, $z60, $z37 
        vcpfftps.l $z44, $z29, $z60, $z44 
        vcpfftps.h $z45, $z29, $z60, $z45 
        vcpfftps.l $z38, $z23, $z61, $z38        
        vcpfftps.h $z39, $z23, $z61, $z39 
        vcpfftps.l $z46, $z31, $z61, $z46       
        vcpfftps.h $z47, $z31, $z61, $z47 
                                             
fft_stage_5_g4:        
##C->A
        vcpfftps.lpre $z16, $z40, $z60, $z32
        vcpfftps.hpre $z24, $z40, $z60, $z32
        vcpfftps.lpre $z20, $z41, $z61, $z33        
        vcpfftps.hpre $z28, $z41, $z61, $z33      
        vcpfftps.lpre $z18, $z42, $z62, $z34
        vcpfftps.hpre $z26, $z42, $z62, $z34
        vcpfftps.lpre $z22, $z43, $z63, $z35        
        vcpfftps.hpre $z30, $z43, $z63, $z35
        
        vcpfftps.lpre $z17, $z44, $z60, $z36
        vcpfftps.hpre $z25, $z44, $z60, $z36
        vcpfftps.lpre $z21, $z45, $z61, $z37        
        vcpfftps.hpre $z29, $z45, $z61, $z37       
        vcpfftps.lpre $z19, $z46, $z62, $z38
        vcpfftps.hpre $z27, $z46, $z62, $z38
        vcpfftps.lpre $z23, $z47, $z63, $z39        
        vcpfftps.hpre $z31, $z47, $z63, $z39
       
        vcpfftps.l $z16, $z40, $z60, $z16
        vcpfftps.l $z20, $z41, $z61, $z20
        vcpfftps.h $z24, $z40, $z60, $z24
        vcpfftps.h $z28, $z41, $z61, $z28
        vcpfftps.l $z18, $z42, $z62, $z18
        vcpfftps.l $z22, $z43, $z63, $z22
        vcpfftps.h $z26, $z42, $z62, $z26
        vcpfftps.h $z30, $z43, $z63, $z30

        vcpfftps.l $z17, $z44, $z60, $z17       
        vcpfftps.l $z21, $z45, $z61, $z21     
        vcpfftps.h $z25, $z44, $z60, $z25       
        vcpfftps.h $z29, $z45, $z61, $z29
        vcpfftps.l $z19, $z46, $z62, $z19       
        vcpfftps.l $z23, $z47, $z63, $z23      
        vcpfftps.h $z27, $z46, $z62, $z27       
        vcpfftps.h $z31, $z47, $z63, $z31

        subu s0, s0, 0x1
        sd   t7, 0xb0(t0)

        bnez s0, fft_cal_1
        nop

##seveth group
1:      bc2f(0, 0xffff)
        nop
fft_stage_1_g7:       
##A->A
        vcpfftps.lpre $z32, $z8 , $z58, $z0 ##1 
        vcpfftps.lpre $z36, $z12, $z58, $z4 ##9
        vcpfftps.lpre $z33, $z9 , $z58, $z1 ##3       
        vcpfftps.lpre $z37, $z13, $z58, $z5 ##11       
        vcpfftps.lpre $z34, $z10, $z58, $z2 ##5
        vcpfftps.lpre $z38, $z14, $z58, $z6 ##13
        vcpfftps.lpre $z35, $z11, $z58, $z3 ##7       
        vcpfftps.lpre $z39, $z15, $z58, $z7 ##15       
                                    
        vcpfftps.hpre $z40, $z8 , $z58, $z0 ##2
        vcpfftps.hpre $z44, $z12, $z58, $z4 ##10
        vcpfftps.hpre $z41, $z9 , $z58, $z1 ##4      
        vcpfftps.hpre $z45, $z13, $z58, $z5 ##12
        vcpfftps.hpre $z42, $z10, $z58, $z2 ##6     
        vcpfftps.hpre $z46, $z14, $z58, $z6 ##14
        vcpfftps.hpre $z43, $z11, $z58, $z3 ##8
        vcpfftps.hpre $z47, $z15, $z58, $z7 ##16

        vcpfftps.l $z0 , $z8 , $z58, $z32
        vcpfftps.l $z4 , $z12, $z58, $z36
        vcpfftps.l $z2 , $z10, $z58, $z34
        vcpfftps.l $z6 , $z14, $z58, $z38
        vcpfftps.l $z1 , $z9 , $z58, $z33
        vcpfftps.l $z5 , $z13, $z58, $z37
        vcpfftps.l $z3 , $z11, $z58, $z35
        vcpfftps.l $z7 , $z15, $z58, $z39
                                         
        vcpfftps.h $z8 , $z8 , $z58, $z40
        vcpfftps.h $z12, $z12, $z58, $z44
        vcpfftps.h $z9 , $z9 , $z58, $z41
        vcpfftps.h $z13, $z13, $z58, $z45
        vcpfftps.h $z10, $z10, $z58, $z42
        vcpfftps.h $z14, $z14, $z58, $z46
        vcpfftps.h $z11, $z11, $z58, $z43
        vcpfftps.h $z15, $z15, $z58, $z47
                                           
fft_stage_2_g7:       
##A->C
        vcpfftps.lpre $z32, $z4 , $z59, $z0   
        vcpfftps.hpre $z36, $z4 , $z59, $z0  
        vcpfftps.lpre $z34, $z6 , $z59, $z2   
        vcpfftps.hpre $z38, $z6 , $z59, $z2     
        vcpfftps.lpre $z33, $z5 , $z59, $z1        
        vcpfftps.hpre $z37, $z5 , $z59, $z1      
        vcpfftps.lpre $z35, $z7 , $z59, $z3       
        vcpfftps.hpre $z39, $z7 , $z59, $z3  

        vcpfftps.lpre $z40, $z12, $z59, $z8 
        vcpfftps.hpre $z44, $z12, $z59, $z8  
        vcpfftps.lpre $z42, $z14, $z59, $z10 
        vcpfftps.hpre $z46, $z14, $z59, $z10 
        vcpfftps.lpre $z41, $z13, $z59, $z9       
        vcpfftps.hpre $z45, $z13, $z59, $z9  
        vcpfftps.lpre $z43, $z15, $z59, $z11       
        vcpfftps.hpre $z47, $z15, $z59, $z11 

        bc2f(6, 0xffff)
        nop
        sd  t7, 0x88(t0)
        nop
        vcpfftps.l $z32, $z4 , $z59, $z32 
        vcpfftps.l $z34, $z6 , $z59, $z34 
        vcpfftps.h $z36, $z4 , $z59, $z36 
        vcpfftps.h $z38, $z6 , $z59, $z38 
        vcpfftps.l $z33, $z5 , $z59, $z33         
        vcpfftps.l $z35, $z7 , $z59, $z35        
        vcpfftps.h $z37, $z5 , $z59, $z37  
        vcpfftps.h $z39, $z7 , $z59, $z39 

        vcpfftps.l $z40, $z12, $z59, $z40 
        vcpfftps.l $z42, $z14, $z59, $z42
        vcpfftps.l $z41, $z13, $z59, $z41         
        vcpfftps.l $z43, $z15, $z59, $z43        
        vcpfftps.h $z44, $z12, $z59, $z44 
        vcpfftps.h $z46, $z14, $z59, $z46 
        vcpfftps.h $z45, $z13, $z59, $z45 
        vcpfftps.h $z47, $z15, $z59, $z47 

fft_stage_3_g7:       
##C->A
        vcpfftps.lpre $z0 , $z34, $z60, $z32  
        vcpfftps.hpre $z2 , $z34, $z60, $z32  
        vcpfftps.lpre $z1 , $z35, $z60, $z33      
        vcpfftps.hpre $z3 , $z35, $z60, $z33     
        vcpfftps.lpre $z4 , $z38, $z60, $z36  
        vcpfftps.hpre $z6 , $z38, $z60, $z36     
        vcpfftps.lpre $z5 , $z39, $z60, $z37       
        vcpfftps.hpre $z7 , $z39, $z60, $z37  
        
        vcpfftps.lpre $z8 , $z42, $z60, $z40  
        vcpfftps.lpre $z9 , $z43, $z60, $z41        
        vcpfftps.hpre $z10, $z42, $z60, $z40 
        vcpfftps.hpre $z11, $z43, $z60, $z41 
        vcpfftps.lpre $z12, $z46, $z60, $z44  
        vcpfftps.hpre $z14, $z46, $z60, $z44  
        vcpfftps.lpre $z13, $z47, $z60, $z45        
        vcpfftps.hpre $z15, $z47, $z60, $z45  

        vcpfftps.l $z0 , $z34, $z60, $z0  
        vcpfftps.l $z1 , $z35, $z60, $z1         
        vcpfftps.h $z2 , $z34, $z60, $z2  
        vcpfftps.h $z3 , $z35, $z60, $z3  
        vcpfftps.l $z4 , $z38, $z60, $z4  
        vcpfftps.l $z5 , $z39, $z60, $z5        
        vcpfftps.l $z8 , $z42, $z60, $z8  
        vcpfftps.l $z9 , $z43, $z60, $z9       

        vcpfftps.h $z6 , $z38, $z60, $z6  
        vcpfftps.h $z7 , $z39, $z60, $z7  
        vcpfftps.h $z10, $z42, $z60, $z10 
        vcpfftps.h $z11, $z43, $z60, $z11 
        vcpfftps.l $z12, $z46, $z60, $z12 
        vcpfftps.l $z13, $z47, $z60, $z13       
        vcpfftps.h $z14, $z46, $z60, $z14 
        vcpfftps.h $z15, $z47, $z60, $z15 
                                             
fft_stage_4_g7:       
##A->C
        vcpfftps.lpre $z32, $z1 , $z60, $z0  
        vcpfftps.hpre $z33, $z1 , $z60, $z0 
        vcpfftps.lpre $z34, $z3 , $z61, $z2        
        vcpfftps.hpre $z35, $z3 , $z61, $z2      
        vcpfftps.lpre $z40, $z9 , $z60, $z8 
        vcpfftps.hpre $z41, $z9 , $z60, $z8  
        vcpfftps.lpre $z36, $z5 , $z60, $z4  
        vcpfftps.hpre $z37, $z5 , $z60, $z4    

        vcpfftps.lpre $z42, $z11, $z61, $z10       
        vcpfftps.hpre $z43, $z11, $z61, $z10
        vcpfftps.lpre $z38, $z7 , $z61, $z6        
        vcpfftps.hpre $z39, $z7 , $z61, $z6 
        vcpfftps.lpre $z44, $z13, $z60, $z12 
        vcpfftps.hpre $z45, $z13, $z60, $z12 
        vcpfftps.lpre $z46, $z15, $z61, $z14       
        vcpfftps.hpre $z47, $z15, $z61, $z14 

        vcpfftps.l $z32, $z1 , $z60, $z32 
        vcpfftps.h $z33, $z1 , $z60, $z33 
        vcpfftps.l $z40, $z9 , $z60, $z40
        vcpfftps.h $z41, $z9 , $z60, $z41
        vcpfftps.l $z34, $z3 , $z61, $z34        
        vcpfftps.h $z35, $z3 , $z61, $z35 
        vcpfftps.l $z42, $z11, $z61, $z42      
        vcpfftps.h $z43, $z11, $z61, $z43 
        
        vcpfftps.l $z36, $z5 , $z60, $z36 
        vcpfftps.h $z37, $z5 , $z60, $z37 
        vcpfftps.l $z44, $z13, $z60, $z44 
        vcpfftps.h $z45, $z13, $z60, $z45
        vcpfftps.l $z38, $z7 , $z61, $z38        
        vcpfftps.h $z39, $z7 , $z61, $z39 
        vcpfftps.l $z46, $z15, $z61, $z46       
        vcpfftps.h $z47, $z15, $z61, $z47 
                                             
fft_stage_5_g7:        
##C->A
        vcpfftps.lpre $z0 , $z40, $z60, $z32
        vcpfftps.lpre $z4 , $z41, $z61, $z33        
        vcpfftps.hpre $z8 , $z40, $z60, $z32
        vcpfftps.hpre $z12, $z41, $z61, $z33      
        vcpfftps.lpre $z2 , $z42, $z62, $z34
        vcpfftps.lpre $z6 , $z43, $z63, $z35       
        vcpfftps.hpre $z10, $z42, $z62, $z34
        vcpfftps.hpre $z14, $z43, $z63, $z35

        vcpfftps.lpre $z1 , $z44, $z60, $z36
        vcpfftps.lpre $z5 , $z45, $z61, $z37        
        vcpfftps.hpre $z9 , $z44, $z60, $z36
        vcpfftps.hpre $z13, $z45, $z61, $z37      
        vcpfftps.lpre $z3 , $z46, $z62, $z38
        vcpfftps.lpre $z7 , $z47, $z63, $z39        
        vcpfftps.hpre $z11, $z46, $z62, $z38
        vcpfftps.hpre $z15, $z47, $z63, $z39
       
        vcpfftps.l $z0 , $z40, $z60, $z0 
        vcpfftps.l $z4 , $z41, $z61, $z4 
        vcpfftps.h $z8 , $z40, $z60, $z8 
        vcpfftps.h $z12, $z41, $z61, $z12
        vcpfftps.l $z2 , $z42, $z62, $z2 
        vcpfftps.l $z6 , $z43, $z63, $z6 
        vcpfftps.h $z10, $z42, $z62, $z10
        vcpfftps.h $z14, $z43, $z63, $z14
                                         
##axi to vec0 for din_B        
        dli  t4, 0x07780080010080          ##times=0x10=16,length=0x80=128bytes,m_stride=0x200=512, arcache=0xf, arcmd=0xe
        dli  t6, 0x000100cf0010c0          ##cycle_r_addr=0xcf=51, r_addr=0xc0=48, r_stride=0x1, seg_pin_count=0, seg_step=0x80

        vcpfftps.l $z1 , $z44, $z60, $z1         
        vcpfftps.l $z5 , $z45, $z61, $z5       
        vcpfftps.h $z9 , $z44, $z60, $z9         
        vcpfftps.h $z13, $z45, $z61, $z13
        vcpfftps.l $z3 , $z46, $z62, $z3         
        vcpfftps.l $z7 , $z47, $z63, $z7        
        vcpfftps.h $z11, $z46, $z62, $z11        
        vcpfftps.h $z15, $z47, $z63, $z15
           
        sd   t7, 0xb0(t0)
##        sq (_t6, _t4, _t0, 2)
        sd  t4, 0x20(t0)
        sd  t6, 0x28(t0)       

##axi to vec0 for din_A         
        dli  s4, 0x107780080010010          ##times=0x10=16,length=0x10=16bytes,m_stride=0x200=512, arcache=0xf, arcmd=0xe, qaddr=0x20=32bytes
        dli  s5, 0x0f204007f001000       ##cycle_r_addr=0x7f=31, r_addr=0x00, r_stride=0x1, seg_pin_count=1, seg_step=0x200, qtimes=0xf=15

##eigth group
1:      bc2f(1, 0xffff)
        nop

start_din_dma_rab2:
##        sq (_s5, _s4, _t0, 1)       
        sd  s4, 0x10(t0)
        sd  s5, 0x18(t0)       
        daddu s2, t1, 0xc0 ##for B

fft_stage_1_g8:       
##A->A
        vcpfftps.lpre $z32, $z24, $z58, $z16##1 
        vcpfftps.lpre $z36, $z28, $z58, $z20##9
        vcpfftps.lpre $z33, $z25, $z58, $z17##3       
        vcpfftps.lpre $z37, $z29, $z58, $z21##11       
        vcpfftps.lpre $z34, $z26, $z58, $z18##5
        vcpfftps.lpre $z38, $z30, $z58, $z22##13
        vcpfftps.lpre $z35, $z27, $z58, $z19##7       
        vcpfftps.lpre $z39, $z31, $z58, $z23##15       
                                    
        vcpfftps.hpre $z40, $z24, $z58, $z16##2
        vcpfftps.hpre $z44, $z28, $z58, $z20##10
        vcpfftps.hpre $z41, $z25, $z58, $z17##4      
        vcpfftps.hpre $z45, $z29, $z58, $z21##12
        vcpfftps.hpre $z42, $z26, $z58, $z18##6     
        vcpfftps.hpre $z46, $z30, $z58, $z22##14
        vcpfftps.hpre $z43, $z27, $z58, $z19##8
        vcpfftps.hpre $z47, $z31, $z58, $z23##16

        vcpfftps.l $z16, $z24, $z58, $z32
        vcpfftps.l $z20, $z28, $z58, $z36
        vcpfftps.l $z18, $z26, $z58, $z34
        vcpfftps.l $z22, $z30, $z58, $z38
        vcpfftps.l $z17, $z25, $z58, $z33
        vcpfftps.l $z21, $z29, $z58, $z37
        vcpfftps.l $z19, $z27, $z58, $z35
        vcpfftps.l $z23, $z31, $z58, $z39
                                         
        vcpfftps.h $z24, $z24, $z58, $z40
        vcpfftps.h $z28, $z28, $z58, $z44
        vcpfftps.h $z25, $z25, $z58, $z41
        vcpfftps.h $z29, $z29, $z58, $z45
        vcpfftps.h $z26, $z26, $z58, $z42
        vcpfftps.h $z30, $z30, $z58, $z46
        vcpfftps.h $z27, $z27, $z58, $z43
        vcpfftps.h $z31, $z31, $z58, $z47
                                           
fft_stage_2_g8:       
##A->C
        vcpfftps.lpre $z32, $z20, $z59, $z16
        vcpfftps.hpre $z36, $z20, $z59, $z16 
        vcpfftps.lpre $z34, $z22, $z59, $z18
        vcpfftps.hpre $z38, $z22, $z59, $z18    
        vcpfftps.lpre $z33, $z21, $z59, $z17
        vcpfftps.hpre $z37, $z21, $z59, $z17     
        vcpfftps.lpre $z35, $z23, $z59, $z19
        vcpfftps.hpre $z39, $z23, $z59, $z19 

        vcpfftps.lpre $z40, $z28, $z59, $z24
        vcpfftps.hpre $z44, $z28, $z59, $z24
        vcpfftps.lpre $z42, $z30, $z59, $z26
        vcpfftps.hpre $z46, $z30, $z59, $z26 
        vcpfftps.lpre $z41, $z29, $z59, $z25
        vcpfftps.hpre $z45, $z29, $z59, $z25 
        vcpfftps.lpre $z43, $z31, $z59, $z27
        vcpfftps.hpre $z47, $z31, $z59, $z27 

        daddu  t4, s2, 0x80
        daddu  s3, t2, 0x10
        bc2f(6, 0xffff)
        nop
        sd   t7, 0x80(t0)
##        sq   (_s2, _t2, _t0, 0)
        sd  t2, 0x00(t0)
        sd  s2, 0x08(t0)       

        vcpfftps.l $z32, $z20, $z59, $z32 
        vcpfftps.l $z34, $z22, $z59, $z34 
        vcpfftps.h $z36, $z20, $z59, $z36 
        vcpfftps.h $z38, $z22, $z59, $z38 
        vcpfftps.l $z33, $z21, $z59, $z33         
        vcpfftps.l $z35, $z23, $z59, $z35        
        vcpfftps.h $z37, $z21, $z59, $z37  
        vcpfftps.h $z39, $z23, $z59, $z39 
        
        vcpfftps.l $z40, $z28, $z59, $z40 
        vcpfftps.l $z42, $z30, $z59, $z42 
        vcpfftps.l $z41, $z29, $z59, $z41         
        vcpfftps.l $z43, $z31, $z59, $z43        
        vcpfftps.h $z44, $z28, $z59, $z44 
        vcpfftps.h $z46, $z30, $z59, $z46 
        vcpfftps.h $z45, $z29, $z59, $z45 
        vcpfftps.h $z47, $z31, $z59, $z47 
                                             
fft_stage_3_g8:       
##C->A
        vcpfftps.lpre $z16, $z34, $z60, $z32  
        vcpfftps.hpre $z18, $z34, $z60, $z32  
        vcpfftps.lpre $z17, $z35, $z60, $z33       
        vcpfftps.hpre $z19, $z35, $z60, $z33      
        vcpfftps.lpre $z20, $z38, $z60, $z36  
        vcpfftps.hpre $z22, $z38, $z60, $z36     
        vcpfftps.lpre $z21, $z39, $z60, $z37      
        vcpfftps.hpre $z23, $z39, $z60, $z37  

        vcpfftps.lpre $z24, $z42, $z60, $z40  
        vcpfftps.lpre $z25, $z43, $z60, $z41        
        vcpfftps.hpre $z26, $z42, $z60, $z40  
        vcpfftps.hpre $z27, $z43, $z60, $z41  
        vcpfftps.lpre $z28, $z46, $z60, $z44  
        vcpfftps.hpre $z30, $z46, $z60, $z44  
        vcpfftps.lpre $z29, $z47, $z60, $z45        
        vcpfftps.hpre $z31, $z47, $z60, $z45  

        vcpfftps.l $z16, $z34, $z60, $z16
        vcpfftps.l $z17, $z35, $z60, $z17      
        vcpfftps.h $z18, $z34, $z60, $z18
        vcpfftps.h $z19, $z35, $z60, $z19
        vcpfftps.l $z20, $z38, $z60, $z20
        vcpfftps.l $z21, $z39, $z60, $z21      
        vcpfftps.l $z24, $z42, $z60, $z24
        vcpfftps.l $z25, $z43, $z60, $z25     

        vcpfftps.h $z22, $z38, $z60, $z22
        vcpfftps.h $z23, $z39, $z60, $z23
        vcpfftps.h $z26, $z42, $z60, $z26
        vcpfftps.h $z27, $z43, $z60, $z27
        vcpfftps.l $z28, $z46, $z60, $z28
        vcpfftps.l $z29, $z47, $z60, $z29     
        vcpfftps.h $z30, $z46, $z60, $z30
        vcpfftps.h $z31, $z47, $z60, $z31
                                             
fft_stage_4_g8:       
##A->C
        vcpfftps.lpre $z32, $z17, $z60, $z16
        vcpfftps.hpre $z33, $z17, $z60, $z16
        vcpfftps.lpre $z34, $z19, $z61, $z18
        vcpfftps.hpre $z35, $z19, $z61, $z18
        vcpfftps.lpre $z40, $z25, $z60, $z24
        vcpfftps.hpre $z41, $z25, $z60, $z24
        vcpfftps.lpre $z36, $z21, $z60, $z20
        vcpfftps.hpre $z37, $z21, $z60, $z20

        vcpfftps.lpre $z42, $z27, $z61, $z26
        vcpfftps.hpre $z43, $z27, $z61, $z26
        vcpfftps.lpre $z38, $z23, $z61, $z22
        vcpfftps.hpre $z39, $z23, $z61, $z22
        vcpfftps.lpre $z44, $z29, $z60, $z28
        vcpfftps.hpre $z45, $z29, $z60, $z28
        vcpfftps.lpre $z46, $z31, $z61, $z30
        vcpfftps.hpre $z47, $z31, $z61, $z30

        vcpfftps.l $z32, $z17, $z60, $z32 
        vcpfftps.h $z33, $z17, $z60, $z33 
        vcpfftps.l $z40, $z25, $z60, $z40 
        vcpfftps.h $z41, $z25, $z60, $z41 
        vcpfftps.l $z34, $z19, $z61, $z34        
        vcpfftps.h $z35, $z19, $z61, $z35
        vcpfftps.l $z42, $z27, $z61, $z42        
        vcpfftps.h $z43, $z27, $z61, $z43 

        vcpfftps.l $z36, $z21, $z60, $z36 
        vcpfftps.h $z37, $z21, $z60, $z37 
        vcpfftps.l $z44, $z29, $z60, $z44 
        vcpfftps.h $z45, $z29, $z60, $z45 
        vcpfftps.l $z38, $z23, $z61, $z38        
        vcpfftps.h $z39, $z23, $z61, $z39 
        vcpfftps.l $z46, $z31, $z61, $z46       
        vcpfftps.h $z47, $z31, $z61, $z47 
                                             
fft_stage_5_g8:        
##C->A
        vcpfftps.lpre $z16, $z40, $z60, $z32
        vcpfftps.hpre $z24, $z40, $z60, $z32
        vcpfftps.lpre $z20, $z41, $z61, $z33        
        vcpfftps.hpre $z28, $z41, $z61, $z33      
        vcpfftps.lpre $z18, $z42, $z62, $z34
        vcpfftps.hpre $z26, $z42, $z62, $z34
        vcpfftps.lpre $z22, $z43, $z63, $z35        
        vcpfftps.hpre $z30, $z43, $z63, $z35
        
        vcpfftps.lpre $z17, $z44, $z60, $z36
        vcpfftps.hpre $z25, $z44, $z60, $z36
        vcpfftps.lpre $z21, $z45, $z61, $z37        
        vcpfftps.hpre $z29, $z45, $z61, $z37       
        vcpfftps.lpre $z19, $z46, $z62, $z38
        vcpfftps.hpre $z27, $z46, $z62, $z38
        vcpfftps.lpre $z23, $z47, $z63, $z39        
        vcpfftps.hpre $z31, $z47, $z63, $z39
       
        vcpfftps.l $z16, $z40, $z60, $z16
        vcpfftps.l $z20, $z41, $z61, $z20
        vcpfftps.h $z24, $z40, $z60, $z24
        vcpfftps.h $z28, $z41, $z61, $z28
        vcpfftps.l $z18, $z42, $z62, $z18
        vcpfftps.l $z22, $z43, $z63, $z22
        vcpfftps.h $z26, $z42, $z62, $z26
        vcpfftps.h $z30, $z43, $z63, $z30

        vcpfftps.l $z17, $z44, $z60, $z17       
        vcpfftps.l $z21, $z45, $z61, $z21     
        vcpfftps.h $z25, $z44, $z60, $z25       
        vcpfftps.h $z29, $z45, $z61, $z29
        vcpfftps.l $z19, $z46, $z62, $z19       
        vcpfftps.l $z23, $z47, $z63, $z23      
        vcpfftps.h $z27, $z46, $z62, $z27       
        vcpfftps.h $z31, $z47, $z63, $z31

        sd   t7, 0xb0(t0)
        sd   t4, 0x38(t0)

##fft_cal_2:
## first group
1:      bc2f(0, 0xffff)
        nop
shuff_EA:
        vpermutqi   $z7 , $z7 , $z16, 0x20
        vpermutqi   $z15, $z15, $z17, 0x20
        bc2f(2, 0xffff)
        nop
fft_stage_6_g1:        
##A->C
        vcpfftps.lpre $z32, $z8 , $z48, $z0 
        vcpfftps.lpre $z40, $z12, $z48, $z4 
        vcpfftps.lpre $z34, $z9 , $z48, $z1         
        vcpfftps.lpre $z42, $z13, $z48, $z5         
        vcpfftps.lpre $z36, $z10, $z48, $z2 
        vcpfftps.lpre $z44, $z14, $z48, $z6 
        vcpfftps.lpre $z38, $z11, $z48, $z3         
        vcpfftps.lpre $z46, $z15, $z48, $z7        

        vcpfftps.hpre $z33, $z8 , $z48, $z0 
        vcpfftps.hpre $z41, $z12, $z48, $z4 
        vcpfftps.hpre $z35, $z9 , $z48, $z1      
        vcpfftps.hpre $z43, $z13, $z48, $z5        
        vcpfftps.hpre $z37, $z10, $z48, $z2 
        vcpfftps.hpre $z45, $z14, $z48, $z6 
        vcpfftps.hpre $z39, $z11, $z48, $z3 
        vcpfftps.hpre $z47, $z15, $z48, $z7 
        
        vcpfftps.l $z32, $z8 , $z48, $z32
        vcpfftps.l $z40, $z12, $z48, $z40
        vcpfftps.l $z36, $z10, $z48, $z36
        vcpfftps.l $z44, $z14, $z48, $z44
        vcpfftps.l $z34, $z9 , $z48, $z34
        vcpfftps.l $z42, $z13, $z48, $z42
        vcpfftps.l $z38, $z11, $z48, $z38
        vcpfftps.l $z46, $z15, $z48, $z46
                                         
        vcpfftps.h $z33, $z8 , $z48, $z33
        vcpfftps.h $z41, $z12, $z48, $z41
        vcpfftps.h $z37, $z10, $z48, $z37
        vcpfftps.h $z45, $z14, $z48, $z45
        vcpfftps.h $z35, $z9 , $z48, $z35
        vcpfftps.h $z43, $z13, $z48, $z43
        vcpfftps.h $z39, $z11, $z48, $z39
        vcpfftps.h $z47, $z15, $z48, $z47
        
fft_stage_7_g1:        
##C->A
        vcpfftps.lpre $z0 , $z40, $z49, $z32
        vcpfftps.hpre $z8 , $z40, $z49, $z32
        vcpfftps.lpre $z4 , $z44, $z49, $z36
        vcpfftps.hpre $z12, $z44, $z49, $z36
        vcpfftps.lpre $z2 , $z42, $z49, $z34
        vcpfftps.hpre $z10, $z42, $z49, $z34
        vcpfftps.lpre $z6 , $z46, $z49, $z38
        vcpfftps.hpre $z14, $z46, $z49, $z38

        vcpfftps.lpre $z1 , $z41, $z49, $z33       
        vcpfftps.lpre $z5 , $z45, $z49, $z37       
        vcpfftps.hpre $z9 , $z41, $z49, $z33       
        vcpfftps.hpre $z13, $z45, $z49, $z37
        vcpfftps.lpre $z3 , $z43, $z49, $z35       
        vcpfftps.lpre $z7 , $z47, $z49, $z39       
        vcpfftps.hpre $z11, $z43, $z49, $z35 
        vcpfftps.hpre $z15, $z47, $z49, $z39 

        bc2f(6, 0xffff)
        nop
conf_dout_dma_2: ##Cr8~Cr9
##vec to axi
        dli  t6, 0x16004007c004000       ##cycle_r_addr=0x7c=31, r_addr=0x0, r_stride=0x4, seg_pin_count=0, seg_step=0x200, fft_clr=0, fft_step=6, wall=1        
        sd   t7, 0x88(t0)
        nop
        
        vcpfftps.l $z0 , $z40, $z49, $z0 
        vcpfftps.h $z8 , $z40, $z49, $z8
        vcpfftps.l $z4 , $z44, $z49, $z4 
        vcpfftps.h $z12, $z44, $z49, $z12
        vcpfftps.l $z2 , $z42, $z49, $z2
        vcpfftps.l $z6 , $z46, $z49, $z6
        vcpfftps.l $z1 , $z41, $z49, $z1        
        vcpfftps.l $z5 , $z45, $z49, $z5       

        vcpfftps.h $z10, $z42, $z49, $z10
        vcpfftps.h $z14, $z46, $z49, $z14
        vcpfftps.l $z3 , $z43, $z49, $z3         
        vcpfftps.l $z7 , $z47, $z49, $z7       
        vcpfftps.h $z9 , $z41, $z49, $z9       
        vcpfftps.h $z13, $z45, $z49, $z13
        vcpfftps.h $z11, $z43, $z49, $z11
        vcpfftps.h $z15, $z47, $z49, $z15

fft_stage_8_g1:     
##A->C
        vcpfftps.lpre $z32, $z4 , $z50, $z0
        vcpfftps.hpre $z36, $z4 , $z50, $z0
        vcpfftps.lpre $z40, $z12, $z51, $z8 
        vcpfftps.hpre $z44, $z12, $z51, $z8
        vcpfftps.lpre $z34, $z6 , $z50, $z2
        vcpfftps.hpre $z38, $z6 , $z50, $z2
        vcpfftps.lpre $z33, $z5 , $z50, $z1        
        vcpfftps.hpre $z37, $z5 , $z50, $z1     

        vcpfftps.lpre $z35, $z7 , $z50, $z3         
        vcpfftps.hpre $z39, $z7 , $z50, $z3
        vcpfftps.lpre $z41, $z13, $z51, $z9         
        vcpfftps.hpre $z45, $z13, $z51, $z9 
        vcpfftps.lpre $z42, $z14, $z51, $z10
        vcpfftps.hpre $z46, $z14, $z51, $z10
        vcpfftps.lpre $z43, $z15, $z51, $z11       
        vcpfftps.hpre $z47, $z15, $z51, $z11

        vcpfftps.l $z32, $z4 , $z50, $z32
        vcpfftps.h $z36, $z4 , $z50, $z36
        vcpfftps.l $z34, $z6 , $z50, $z34
        vcpfftps.h $z38, $z6 , $z50, $z38
        vcpfftps.l $z33, $z5 , $z50, $z33      
        vcpfftps.h $z37, $z5 , $z50, $z37    
        vcpfftps.l $z35, $z7 , $z50, $z35    
        vcpfftps.h $z39, $z7 , $z50, $z39

        vcpfftps.l $z40, $z12, $z51, $z40
        vcpfftps.h $z44, $z12, $z51, $z44
        vcpfftps.l $z42, $z14, $z51, $z42
        vcpfftps.h $z46, $z14, $z51, $z46
        vcpfftps.l $z41, $z13, $z51, $z41      
        vcpfftps.h $z45, $z13, $z51, $z45
        vcpfftps.l $z43, $z15, $z51, $z43       
        vcpfftps.h $z47, $z15, $z51, $z47

        sd  t7, 0x90(t0)
        nop
        bc2f(4, 0xffff)
        nop

fft_stage_9_g1:              
##C->A
        vcpfftps.lpre $z0 , $z34, $z52, $z32
        vcpfftps.hpre $z2 , $z34, $z52, $z32
        vcpfftps.lpre $z4 , $z38, $z53, $z36
        vcpfftps.hpre $z6 , $z38, $z53, $z36
        vcpfftps.lpre $z1 , $z35, $z52, $z33         
        vcpfftps.hpre $z3 , $z35, $z52, $z33      
        vcpfftps.lpre $z5 , $z39, $z53, $z37      
        vcpfftps.hpre $z7 , $z39, $z53, $z37

        vcpfftps.lpre $z8 , $z42, $z54, $z40
        vcpfftps.hpre $z10, $z42, $z54, $z40
        vcpfftps.lpre $z12, $z46, $z55, $z44
        vcpfftps.hpre $z14, $z46, $z55, $z44
        vcpfftps.lpre $z9 , $z43, $z54, $z41        
        vcpfftps.hpre $z11, $z43, $z54, $z41      
        vcpfftps.lpre $z13, $z47, $z55, $z45        
        vcpfftps.hpre $z15, $z47, $z55, $z45

        vcpfftps.l $z0 , $z34, $z52, $z0 
        vcpfftps.h $z2 , $z34, $z52, $z2 
        vcpfftps.l $z1 , $z35, $z52, $z1         
        vcpfftps.h $z3 , $z35, $z52, $z3        
        vcpfftps.l $z4 , $z38, $z53, $z4 
        vcpfftps.h $z6 , $z38, $z53, $z6 
        vcpfftps.l $z5 , $z39, $z53, $z5       
        vcpfftps.h $z7 , $z39, $z53, $z7 

        vcpfftps.l $z8 , $z42, $z54, $z8 
        vcpfftps.h $z10, $z42, $z54, $z10
        vcpfftps.l $z9 , $z43, $z54, $z9         
        vcpfftps.h $z11, $z43, $z54, $z11       
        vcpfftps.l $z12, $z46, $z55, $z12
        vcpfftps.h $z14, $z46, $z55, $z14
        vcpfftps.l $z13, $z47, $z55, $z13        
        vcpfftps.h $z15, $z47, $z55, $z15

fft_stage_10_g1:        
##A->A
        vcpfftps.lpre $z32, $z1 , $z56, $z0 
        vcpfftps.hpre $z33, $z1 , $z56, $z0
        vcpfftps.lpre $z34, $z3 , $z57, $z2         
        vcpfftps.hpre $z35, $z3 , $z57, $z2        
        vcpfftps.lpre $z36, $z5 , $z58, $z4 
        vcpfftps.hpre $z37, $z5 , $z58, $z4 
        vcpfftps.lpre $z38, $z7 , $z59, $z6        
        vcpfftps.hpre $z39, $z7 , $z59, $z6

        vcpfftps.lpre $z40, $z9 , $z60, $z8 
        vcpfftps.hpre $z41, $z9 , $z60, $z8 
        vcpfftps.lpre $z42, $z11, $z61, $z10        
        vcpfftps.hpre $z43, $z11, $z61, $z10       
        vcpfftps.lpre $z44, $z13, $z62, $z12
        vcpfftps.hpre $z45, $z13, $z62, $z12
        vcpfftps.lpre $z46, $z15, $z63, $z14        
        vcpfftps.hpre $z47, $z15, $z63, $z14

        vcpfftps.l $z0 , $z1 , $z56, $z32
        vcpfftps.h $z1 , $z1 , $z56, $z33
        vcpfftps.l $z2 , $z3 , $z57, $z34       
        vcpfftps.h $z3 , $z3 , $z57, $z35       
        vcpfftps.l $z4 , $z5 , $z58, $z36
        vcpfftps.h $z5 , $z5 , $z58, $z37
        vcpfftps.l $z6 , $z7 , $z59, $z38      
        vcpfftps.h $z7 , $z7 , $z59, $z39

        vcpfftps.l $z8 , $z9 , $z60, $z40
        vcpfftps.h $z9 , $z9 , $z60, $z41
        vcpfftps.l $z10, $z11, $z61, $z42        
        vcpfftps.h $z11, $z11, $z61, $z43       
        vcpfftps.l $z12, $z13, $z62, $z44
        vcpfftps.h $z13, $z13, $z62, $z45
        vcpfftps.l $z14, $z15, $z63, $z46       
        vcpfftps.h $z15, $z15, $z63, $z47

        sd   t7, 0xb0(t0)
        sd   t6, 0x48(t0)
        sd   t5, 0x30(t0)
        sd   t7, 0xa0(t0)
       
##second group
1:      bc2f(1, 0xffff)
        nop
        bc2f(2, 0xffff)
        nop
fft_stage_6_g2:        
##B->C
        vcpfftps.lpre $z32, $z24, $z48, $z16
        vcpfftps.lpre $z40, $z28, $z48, $z20
        vcpfftps.lpre $z34, $z25, $z48, $z17        
        vcpfftps.lpre $z42, $z29, $z48, $z21        
        vcpfftps.lpre $z36, $z26, $z48, $z18
        vcpfftps.lpre $z44, $z30, $z48, $z22
        vcpfftps.lpre $z38, $z27, $z48, $z19        
        vcpfftps.lpre $z46, $z31, $z48, $z23       

        vcpfftps.hpre $z33, $z24, $z48, $z16
        vcpfftps.hpre $z41, $z28, $z48, $z20
        vcpfftps.hpre $z35, $z25, $z48, $z17    
        vcpfftps.hpre $z43, $z29, $z48, $z21      
        vcpfftps.hpre $z37, $z26, $z48, $z18
        vcpfftps.hpre $z45, $z30, $z48, $z22
        vcpfftps.hpre $z39, $z27, $z48, $z19
        vcpfftps.hpre $z47, $z31, $z48, $z23
        
        vcpfftps.l $z32, $z24, $z48, $z32
        vcpfftps.l $z40, $z28, $z48, $z40
        vcpfftps.l $z36, $z26, $z48, $z36
        vcpfftps.l $z44, $z30, $z48, $z44
        vcpfftps.l $z34, $z25, $z48, $z34
        vcpfftps.l $z42, $z29, $z48, $z42
        vcpfftps.l $z38, $z27, $z48, $z38
        vcpfftps.l $z46, $z31, $z48, $z46
                                         
        vcpfftps.h $z33, $z24, $z48, $z33
        vcpfftps.h $z41, $z28, $z48, $z41
        vcpfftps.h $z37, $z26, $z48, $z37
        vcpfftps.h $z45, $z30, $z48, $z45
        vcpfftps.h $z35, $z25, $z48, $z35
        vcpfftps.h $z43, $z29, $z48, $z43
        vcpfftps.h $z39, $z27, $z48, $z39
        vcpfftps.h $z47, $z31, $z48, $z47
        
fft_stage_7_g2:        
##C->B
        vcpfftps.lpre $z16, $z40, $z49, $z32
        vcpfftps.hpre $z24, $z40, $z49, $z32
        vcpfftps.lpre $z20, $z44, $z49, $z36
        vcpfftps.hpre $z28, $z44, $z49, $z36
        vcpfftps.lpre $z18, $z42, $z49, $z34
        vcpfftps.hpre $z26, $z42, $z49, $z34
        vcpfftps.lpre $z22, $z46, $z49, $z38
        vcpfftps.hpre $z30, $z46, $z49, $z38

        vcpfftps.lpre $z17, $z41, $z49, $z33       
        vcpfftps.lpre $z21, $z45, $z49, $z37       
        vcpfftps.hpre $z25, $z41, $z49, $z33       
        vcpfftps.hpre $z29, $z45, $z49, $z37
        vcpfftps.lpre $z19, $z43, $z49, $z35       
        vcpfftps.lpre $z23, $z47, $z49, $z39       
        vcpfftps.hpre $z27, $z43, $z49, $z35
        vcpfftps.hpre $z31, $z47, $z49, $z39 

        bc2f(6, 0xffff)
        nop
        sd  t7, 0x80(t0)
        nop
        
        vcpfftps.l $z16, $z40, $z49, $z16
        vcpfftps.h $z24, $z40, $z49, $z24
        vcpfftps.l $z20, $z44, $z49, $z20
        vcpfftps.h $z28, $z44, $z49, $z28
        vcpfftps.l $z18, $z42, $z49, $z18
        vcpfftps.l $z22, $z46, $z49, $z22
        vcpfftps.l $z17, $z41, $z49, $z17       
        vcpfftps.l $z21, $z45, $z49, $z21    

        vcpfftps.h $z26, $z42, $z49, $z26
        vcpfftps.h $z30, $z46, $z49, $z30
        vcpfftps.l $z19, $z43, $z49, $z19       
        vcpfftps.l $z23, $z47, $z49, $z23     
        vcpfftps.h $z25, $z41, $z49, $z25     
        vcpfftps.h $z29, $z45, $z49, $z29
        vcpfftps.h $z27, $z43, $z49, $z27
        vcpfftps.h $z31, $z47, $z49, $z31

fft_stage_8_g2:     
##B->C
        vcpfftps.lpre $z32, $z20, $z50, $z16
        vcpfftps.hpre $z36, $z20, $z50, $z16
        vcpfftps.lpre $z40, $z28, $z51, $z24
        vcpfftps.hpre $z44, $z28, $z51, $z24
        vcpfftps.lpre $z34, $z22, $z50, $z18
        vcpfftps.hpre $z38, $z22, $z50, $z18
        vcpfftps.lpre $z33, $z21, $z50, $z17       
        vcpfftps.hpre $z37, $z21, $z50, $z17    

        vcpfftps.lpre $z35, $z23, $z50, $z19        
        vcpfftps.hpre $z39, $z23, $z50, $z19
        vcpfftps.lpre $z41, $z29, $z51, $z25        
        vcpfftps.hpre $z45, $z29, $z51, $z25
        vcpfftps.lpre $z42, $z30, $z51, $z26
        vcpfftps.hpre $z46, $z30, $z51, $z26
        vcpfftps.lpre $z43, $z31, $z51, $z27       
        vcpfftps.hpre $z47, $z31, $z51, $z27

        vcpfftps.l $z32, $z20, $z50, $z32
        vcpfftps.h $z36, $z20, $z50, $z36
        vcpfftps.l $z34, $z22, $z50, $z34
        vcpfftps.h $z38, $z22, $z50, $z38
        vcpfftps.l $z33, $z21, $z50, $z33      
        vcpfftps.h $z37, $z21, $z50, $z37   
        vcpfftps.l $z35, $z23, $z50, $z35     
        vcpfftps.h $z39, $z23, $z50, $z39

        vcpfftps.l $z40, $z28, $z51, $z40
        vcpfftps.h $z44, $z28, $z51, $z44
        vcpfftps.l $z42, $z30, $z51, $z42
        vcpfftps.h $z46, $z30, $z51, $z46
        vcpfftps.l $z41, $z29, $z51, $z41      
        vcpfftps.h $z45, $z29, $z51, $z45
        vcpfftps.l $z43, $z31, $z51, $z43       
        vcpfftps.h $z47, $z31, $z51, $z47

        sd  t7, 0x90(t0)
        nop
        bc2f(4, 0xffff)
        nop

fft_stage_9_g2:              
##C->A
        vcpfftps.lpre $z16, $z34, $z52, $z32
        vcpfftps.hpre $z18, $z34, $z52, $z32
        vcpfftps.lpre $z20, $z38, $z53, $z36
        vcpfftps.hpre $z22, $z38, $z53, $z36
        vcpfftps.lpre $z17, $z35, $z52, $z33         
        vcpfftps.hpre $z19, $z35, $z52, $z33      
        vcpfftps.lpre $z21, $z39, $z53, $z37       
        vcpfftps.hpre $z23, $z39, $z53, $z37

        vcpfftps.lpre $z24, $z42, $z54, $z40
        vcpfftps.hpre $z26, $z42, $z54, $z40
        vcpfftps.lpre $z28, $z46, $z55, $z44
        vcpfftps.hpre $z30, $z46, $z55, $z44
        vcpfftps.lpre $z25, $z43, $z54, $z41        
        vcpfftps.hpre $z27, $z43, $z54, $z41      
        vcpfftps.lpre $z29, $z47, $z55, $z45        
        vcpfftps.hpre $z31, $z47, $z55, $z45

        vcpfftps.l $z16, $z34, $z52, $z16
        vcpfftps.h $z18, $z34, $z52, $z18
        vcpfftps.l $z17, $z35, $z52, $z17       
        vcpfftps.h $z19, $z35, $z52, $z19      
        vcpfftps.l $z20, $z38, $z53, $z20
        vcpfftps.h $z22, $z38, $z53, $z22
        vcpfftps.l $z21, $z39, $z53, $z21      
        vcpfftps.h $z23, $z39, $z53, $z23

        vcpfftps.l $z24, $z42, $z54, $z24
        vcpfftps.h $z26, $z42, $z54, $z26
        vcpfftps.l $z25, $z43, $z54, $z25       
        vcpfftps.h $z27, $z43, $z54, $z27      
        vcpfftps.l $z28, $z46, $z55, $z28
        vcpfftps.h $z30, $z46, $z55, $z30
        vcpfftps.l $z29, $z47, $z55, $z29       
        vcpfftps.h $z31, $z47, $z55, $z31

fft_stage_10_g2:        
##B->B
        vcpfftps.lpre $z32, $z17, $z56, $z16
        vcpfftps.hpre $z33, $z17, $z56, $z16
        vcpfftps.lpre $z34, $z19, $z57, $z18        
        vcpfftps.hpre $z35, $z19, $z57, $z18      
        vcpfftps.lpre $z36, $z21, $z58, $z20
        vcpfftps.hpre $z37, $z21, $z58, $z20
        vcpfftps.lpre $z38, $z23, $z59, $z22      
        vcpfftps.hpre $z39, $z23, $z59, $z22

        vcpfftps.lpre $z40, $z25, $z60, $z24
        vcpfftps.hpre $z41, $z25, $z60, $z24
        vcpfftps.lpre $z42, $z27, $z61, $z26        
        vcpfftps.hpre $z43, $z27, $z61, $z26      
        vcpfftps.lpre $z44, $z29, $z62, $z28
        vcpfftps.hpre $z45, $z29, $z62, $z28
        vcpfftps.lpre $z46, $z31, $z63, $z30        
        vcpfftps.hpre $z47, $z31, $z63, $z30

        vcpfftps.l $z16, $z17, $z56, $z32
        vcpfftps.h $z17, $z17, $z56, $z33
        vcpfftps.l $z18, $z19, $z57, $z34       
        vcpfftps.h $z19, $z19, $z57, $z35       
        vcpfftps.l $z20, $z21, $z58, $z36
        vcpfftps.h $z21, $z21, $z58, $z37
        vcpfftps.l $z22, $z23, $z59, $z38       
        vcpfftps.h $z23, $z23, $z59, $z39

        vcpfftps.l $z24, $z25, $z60, $z40
        vcpfftps.h $z25, $z25, $z60, $z41
        vcpfftps.l $z26, $z27, $z61, $z42        
        vcpfftps.h $z27, $z27, $z61, $z43       
        vcpfftps.l $z28, $z29, $z62, $z44
        vcpfftps.h $z29, $z29, $z62, $z45
        vcpfftps.l $z30, $z31, $z63, $z46       
        vcpfftps.h $z31, $z31, $z63, $z47

        sd  t7, 0xb0(t0)
        sd  t7, 0xa0(t0)
        
        li  s0, 0x2
        nop
        
fft_cal_2_1:
##fifth group
1:      bc2f(0, 0xffff)
        nop
        bc2f(2, 0xffff)
        nop
fft_stage_6_g3:        
##A->C
        vcpfftps.lpre $z32, $z8 , $z48, $z0 
        vcpfftps.lpre $z40, $z12, $z48, $z4 
        vcpfftps.lpre $z34, $z9 , $z48, $z1         
        vcpfftps.lpre $z42, $z13, $z48, $z5         
        vcpfftps.lpre $z36, $z10, $z48, $z2 
        vcpfftps.lpre $z44, $z14, $z48, $z6 
        vcpfftps.lpre $z38, $z11, $z48, $z3         
        vcpfftps.lpre $z46, $z15, $z48, $z7        

        vcpfftps.hpre $z33, $z8 , $z48, $z0 
        vcpfftps.hpre $z41, $z12, $z48, $z4 
        vcpfftps.hpre $z35, $z9 , $z48, $z1      
        vcpfftps.hpre $z43, $z13, $z48, $z5        
        vcpfftps.hpre $z37, $z10, $z48, $z2 
        vcpfftps.hpre $z45, $z14, $z48, $z6 
        vcpfftps.hpre $z39, $z11, $z48, $z3 
        vcpfftps.hpre $z47, $z15, $z48, $z7 
        
        vcpfftps.l $z32, $z8 , $z48, $z32
        vcpfftps.l $z40, $z12, $z48, $z40
        vcpfftps.l $z36, $z10, $z48, $z36
        vcpfftps.l $z44, $z14, $z48, $z44
        vcpfftps.l $z34, $z9 , $z48, $z34
        vcpfftps.l $z42, $z13, $z48, $z42
        vcpfftps.l $z38, $z11, $z48, $z38
        vcpfftps.l $z46, $z15, $z48, $z46
                                         
        vcpfftps.h $z33, $z8 , $z48, $z33
        vcpfftps.h $z41, $z12, $z48, $z41
        vcpfftps.h $z37, $z10, $z48, $z37
        vcpfftps.h $z45, $z14, $z48, $z45
        vcpfftps.h $z35, $z9 , $z48, $z35
        vcpfftps.h $z43, $z13, $z48, $z43
        vcpfftps.h $z39, $z11, $z48, $z39
        vcpfftps.h $z47, $z15, $z48, $z47
        
fft_stage_7_g3:        
##C->A
        vcpfftps.lpre $z0 , $z40, $z49, $z32
        vcpfftps.hpre $z8 , $z40, $z49, $z32
        vcpfftps.lpre $z4 , $z44, $z49, $z36
        vcpfftps.hpre $z12, $z44, $z49, $z36
        vcpfftps.lpre $z2 , $z42, $z49, $z34
        vcpfftps.hpre $z10, $z42, $z49, $z34
        vcpfftps.lpre $z6 , $z46, $z49, $z38
        vcpfftps.hpre $z14, $z46, $z49, $z38

        vcpfftps.lpre $z1 , $z41, $z49, $z33       
        vcpfftps.lpre $z5 , $z45, $z49, $z37       
        vcpfftps.hpre $z9 , $z41, $z49, $z33       
        vcpfftps.hpre $z13, $z45, $z49, $z37
        vcpfftps.lpre $z3 , $z43, $z49, $z35       
        vcpfftps.lpre $z7 , $z47, $z49, $z39       
        vcpfftps.hpre $z11, $z43, $z49, $z35 
        vcpfftps.hpre $z15, $z47, $z49, $z39 

        bc2f(6, 0xffff)
        nop
        sd  t7, 0x88(t0)
        nop
        
        vcpfftps.l $z0 , $z40, $z49, $z0 
        vcpfftps.h $z8 , $z40, $z49, $z8
        vcpfftps.l $z4 , $z44, $z49, $z4 
        vcpfftps.h $z12, $z44, $z49, $z12
        vcpfftps.l $z2 , $z42, $z49, $z2
        vcpfftps.l $z6 , $z46, $z49, $z6
        vcpfftps.l $z1 , $z41, $z49, $z1        
        vcpfftps.l $z5 , $z45, $z49, $z5       

        vcpfftps.h $z10, $z42, $z49, $z10
        vcpfftps.h $z14, $z46, $z49, $z14
        vcpfftps.l $z3 , $z43, $z49, $z3         
        vcpfftps.l $z7 , $z47, $z49, $z7       
        vcpfftps.h $z9 , $z41, $z49, $z9       
        vcpfftps.h $z13, $z45, $z49, $z13
        vcpfftps.h $z11, $z43, $z49, $z11
        vcpfftps.h $z15, $z47, $z49, $z15

fft_stage_8_g3:     
##A->C
        vcpfftps.lpre $z32, $z4 , $z50, $z0
        vcpfftps.hpre $z36, $z4 , $z50, $z0
        vcpfftps.lpre $z40, $z12, $z51, $z8 
        vcpfftps.hpre $z44, $z12, $z51, $z8
        vcpfftps.lpre $z34, $z6 , $z50, $z2
        vcpfftps.hpre $z38, $z6 , $z50, $z2
        vcpfftps.lpre $z33, $z5 , $z50, $z1        
        vcpfftps.hpre $z37, $z5 , $z50, $z1     

        vcpfftps.lpre $z35, $z7 , $z50, $z3         
        vcpfftps.hpre $z39, $z7 , $z50, $z3
        vcpfftps.lpre $z41, $z13, $z51, $z9         
        vcpfftps.hpre $z45, $z13, $z51, $z9 
        vcpfftps.lpre $z42, $z14, $z51, $z10
        vcpfftps.hpre $z46, $z14, $z51, $z10
        vcpfftps.lpre $z43, $z15, $z51, $z11       
        vcpfftps.hpre $z47, $z15, $z51, $z11

        vcpfftps.l $z32, $z4 , $z50, $z32
        vcpfftps.h $z36, $z4 , $z50, $z36
        vcpfftps.l $z34, $z6 , $z50, $z34
        vcpfftps.h $z38, $z6 , $z50, $z38
        vcpfftps.l $z33, $z5 , $z50, $z33      
        vcpfftps.h $z37, $z5 , $z50, $z37    
        vcpfftps.l $z35, $z7 , $z50, $z35    
        vcpfftps.h $z39, $z7 , $z50, $z39

        vcpfftps.l $z40, $z12, $z51, $z40
        vcpfftps.h $z44, $z12, $z51, $z44
        vcpfftps.l $z42, $z14, $z51, $z42
        vcpfftps.h $z46, $z14, $z51, $z46
        vcpfftps.l $z41, $z13, $z51, $z41      
        vcpfftps.h $z45, $z13, $z51, $z45
        vcpfftps.l $z43, $z15, $z51, $z43       
        vcpfftps.h $z47, $z15, $z51, $z47

        sd  t7, 0x90(t0)
        nop
        bc2f(4, 0xffff)
        nop

fft_stage_9_g3:              
##C->A
        vcpfftps.lpre $z0 , $z34, $z52, $z32
        vcpfftps.hpre $z2 , $z34, $z52, $z32
        vcpfftps.lpre $z4 , $z38, $z53, $z36
        vcpfftps.hpre $z6 , $z38, $z53, $z36
        vcpfftps.lpre $z1 , $z35, $z52, $z33         
        vcpfftps.hpre $z3 , $z35, $z52, $z33      
        vcpfftps.lpre $z5 , $z39, $z53, $z37      
        vcpfftps.hpre $z7 , $z39, $z53, $z37

        vcpfftps.lpre $z8 , $z42, $z54, $z40
        vcpfftps.hpre $z10, $z42, $z54, $z40
        vcpfftps.lpre $z12, $z46, $z55, $z44
        vcpfftps.hpre $z14, $z46, $z55, $z44
        vcpfftps.lpre $z9 , $z43, $z54, $z41        
        vcpfftps.hpre $z11, $z43, $z54, $z41      
        vcpfftps.lpre $z13, $z47, $z55, $z45        
        vcpfftps.hpre $z15, $z47, $z55, $z45

        vcpfftps.l $z0 , $z34, $z52, $z0 
        vcpfftps.h $z2 , $z34, $z52, $z2 
        vcpfftps.l $z1 , $z35, $z52, $z1         
        vcpfftps.h $z3 , $z35, $z52, $z3        
        vcpfftps.l $z4 , $z38, $z53, $z4 
        vcpfftps.h $z6 , $z38, $z53, $z6 
        vcpfftps.l $z5 , $z39, $z53, $z5       
        vcpfftps.h $z7 , $z39, $z53, $z7 

        vcpfftps.l $z8 , $z42, $z54, $z8 
        vcpfftps.h $z10, $z42, $z54, $z10
        vcpfftps.l $z9 , $z43, $z54, $z9         
        vcpfftps.h $z11, $z43, $z54, $z11       
        vcpfftps.l $z12, $z46, $z55, $z12
        vcpfftps.h $z14, $z46, $z55, $z14
        vcpfftps.l $z13, $z47, $z55, $z13        
        vcpfftps.h $z15, $z47, $z55, $z15

fft_stage_10_g3:        
##A->A
        vcpfftps.lpre $z32, $z1 , $z56, $z0 
        vcpfftps.hpre $z33, $z1 , $z56, $z0
        vcpfftps.lpre $z34, $z3 , $z57, $z2         
        vcpfftps.hpre $z35, $z3 , $z57, $z2        
        vcpfftps.lpre $z36, $z5 , $z58, $z4 
        vcpfftps.hpre $z37, $z5 , $z58, $z4 
        vcpfftps.lpre $z38, $z7 , $z59, $z6        
        vcpfftps.hpre $z39, $z7 , $z59, $z6

        vcpfftps.lpre $z40, $z9 , $z60, $z8 
        vcpfftps.hpre $z41, $z9 , $z60, $z8 
        vcpfftps.lpre $z42, $z11, $z61, $z10        
        vcpfftps.hpre $z43, $z11, $z61, $z10       
        vcpfftps.lpre $z44, $z13, $z62, $z12
        vcpfftps.hpre $z45, $z13, $z62, $z12
        vcpfftps.lpre $z46, $z15, $z63, $z14        
        vcpfftps.hpre $z47, $z15, $z63, $z14

        vcpfftps.l $z0 , $z1 , $z56, $z32
        vcpfftps.h $z1 , $z1 , $z56, $z33
        vcpfftps.l $z2 , $z3 , $z57, $z34       
        vcpfftps.h $z3 , $z3 , $z57, $z35       
        vcpfftps.l $z4 , $z5 , $z58, $z36
        vcpfftps.h $z5 , $z5 , $z58, $z37
        vcpfftps.l $z6 , $z7 , $z59, $z38      
        vcpfftps.h $z7 , $z7 , $z59, $z39

        vcpfftps.l $z8 , $z9 , $z60, $z40
        vcpfftps.h $z9 , $z9 , $z60, $z41
        vcpfftps.l $z10, $z11, $z61, $z42        
        vcpfftps.h $z11, $z11, $z61, $z43       
        vcpfftps.l $z12, $z13, $z62, $z44
        vcpfftps.h $z13, $z13, $z62, $z45
        vcpfftps.l $z14, $z15, $z63, $z46       
        vcpfftps.h $z15, $z15, $z63, $z47

        sd  t7, 0xb0(t0)
        sd  t7, 0xa0(t0)
       
##fourth group
1:      bc2f(1, 0xffff)
        nop
        bc2f(2, 0xffff)
        nop
fft_stage_6_g4:        
##B->C
        vcpfftps.lpre $z32, $z24, $z48, $z16
        vcpfftps.lpre $z40, $z28, $z48, $z20
        vcpfftps.lpre $z34, $z25, $z48, $z17        
        vcpfftps.lpre $z42, $z29, $z48, $z21        
        vcpfftps.lpre $z36, $z26, $z48, $z18
        vcpfftps.lpre $z44, $z30, $z48, $z22
        vcpfftps.lpre $z38, $z27, $z48, $z19        
        vcpfftps.lpre $z46, $z31, $z48, $z23       

        vcpfftps.hpre $z33, $z24, $z48, $z16
        vcpfftps.hpre $z41, $z28, $z48, $z20
        vcpfftps.hpre $z35, $z25, $z48, $z17    
        vcpfftps.hpre $z43, $z29, $z48, $z21      
        vcpfftps.hpre $z37, $z26, $z48, $z18
        vcpfftps.hpre $z45, $z30, $z48, $z22
        vcpfftps.hpre $z39, $z27, $z48, $z19
        vcpfftps.hpre $z47, $z31, $z48, $z23
        
        vcpfftps.l $z32, $z24, $z48, $z32
        vcpfftps.l $z40, $z28, $z48, $z40
        vcpfftps.l $z36, $z26, $z48, $z36
        vcpfftps.l $z44, $z30, $z48, $z44
        vcpfftps.l $z34, $z25, $z48, $z34
        vcpfftps.l $z42, $z29, $z48, $z42
        vcpfftps.l $z38, $z27, $z48, $z38
        vcpfftps.l $z46, $z31, $z48, $z46
                                         
        vcpfftps.h $z33, $z24, $z48, $z33
        vcpfftps.h $z41, $z28, $z48, $z41
        vcpfftps.h $z37, $z26, $z48, $z37
        vcpfftps.h $z45, $z30, $z48, $z45
        vcpfftps.h $z35, $z25, $z48, $z35
        vcpfftps.h $z43, $z29, $z48, $z43
        vcpfftps.h $z39, $z27, $z48, $z39
        vcpfftps.h $z47, $z31, $z48, $z47
        
fft_stage_7_g4:        
##C->B
        vcpfftps.lpre $z16, $z40, $z49, $z32
        vcpfftps.hpre $z24, $z40, $z49, $z32
        vcpfftps.lpre $z20, $z44, $z49, $z36
        vcpfftps.hpre $z28, $z44, $z49, $z36
        vcpfftps.lpre $z18, $z42, $z49, $z34
        vcpfftps.hpre $z26, $z42, $z49, $z34
        vcpfftps.lpre $z22, $z46, $z49, $z38
        vcpfftps.hpre $z30, $z46, $z49, $z38

        vcpfftps.lpre $z17, $z41, $z49, $z33       
        vcpfftps.lpre $z21, $z45, $z49, $z37       
        vcpfftps.hpre $z25, $z41, $z49, $z33       
        vcpfftps.hpre $z29, $z45, $z49, $z37
        vcpfftps.lpre $z19, $z43, $z49, $z35       
        vcpfftps.lpre $z23, $z47, $z49, $z39       
        vcpfftps.hpre $z27, $z43, $z49, $z35
        vcpfftps.hpre $z31, $z47, $z49, $z39 

        bc2f(6, 0xffff)
        nop
        sd  t7, 0x80(t0)
        nop
        
        vcpfftps.l $z16, $z40, $z49, $z16
        vcpfftps.h $z24, $z40, $z49, $z24
        vcpfftps.l $z20, $z44, $z49, $z20
        vcpfftps.h $z28, $z44, $z49, $z28
        vcpfftps.l $z18, $z42, $z49, $z18
        vcpfftps.l $z22, $z46, $z49, $z22
        vcpfftps.l $z17, $z41, $z49, $z17       
        vcpfftps.l $z21, $z45, $z49, $z21    

        vcpfftps.h $z26, $z42, $z49, $z26
        vcpfftps.h $z30, $z46, $z49, $z30
        vcpfftps.l $z19, $z43, $z49, $z19       
        vcpfftps.l $z23, $z47, $z49, $z23     
        vcpfftps.h $z25, $z41, $z49, $z25     
        vcpfftps.h $z29, $z45, $z49, $z29
        vcpfftps.h $z27, $z43, $z49, $z27
        vcpfftps.h $z31, $z47, $z49, $z31

fft_stage_8_g4:     
##B->C
        vcpfftps.lpre $z32, $z20, $z50, $z16
        vcpfftps.hpre $z36, $z20, $z50, $z16
        vcpfftps.lpre $z40, $z28, $z51, $z24
        vcpfftps.hpre $z44, $z28, $z51, $z24
        vcpfftps.lpre $z34, $z22, $z50, $z18
        vcpfftps.hpre $z38, $z22, $z50, $z18
        vcpfftps.lpre $z33, $z21, $z50, $z17       
        vcpfftps.hpre $z37, $z21, $z50, $z17    

        vcpfftps.lpre $z35, $z23, $z50, $z19        
        vcpfftps.hpre $z39, $z23, $z50, $z19
        vcpfftps.lpre $z41, $z29, $z51, $z25        
        vcpfftps.hpre $z45, $z29, $z51, $z25
        vcpfftps.lpre $z42, $z30, $z51, $z26
        vcpfftps.hpre $z46, $z30, $z51, $z26
        vcpfftps.lpre $z43, $z31, $z51, $z27       
        vcpfftps.hpre $z47, $z31, $z51, $z27

        vcpfftps.l $z32, $z20, $z50, $z32
        vcpfftps.h $z36, $z20, $z50, $z36
        vcpfftps.l $z34, $z22, $z50, $z34
        vcpfftps.h $z38, $z22, $z50, $z38
        vcpfftps.l $z33, $z21, $z50, $z33      
        vcpfftps.h $z37, $z21, $z50, $z37   
        vcpfftps.l $z35, $z23, $z50, $z35     
        vcpfftps.h $z39, $z23, $z50, $z39

        vcpfftps.l $z40, $z28, $z51, $z40
        vcpfftps.h $z44, $z28, $z51, $z44
        vcpfftps.l $z42, $z30, $z51, $z42
        vcpfftps.h $z46, $z30, $z51, $z46
        vcpfftps.l $z41, $z29, $z51, $z41      
        vcpfftps.h $z45, $z29, $z51, $z45
        vcpfftps.l $z43, $z31, $z51, $z43       
        vcpfftps.h $z47, $z31, $z51, $z47

        sd  t7, 0x90(t0)
        nop
        bc2f(4, 0xffff)
        nop

fft_stage_9_g4:              
##C->A
        vcpfftps.lpre $z16, $z34, $z52, $z32
        vcpfftps.hpre $z18, $z34, $z52, $z32
        vcpfftps.lpre $z20, $z38, $z53, $z36
        vcpfftps.hpre $z22, $z38, $z53, $z36
        vcpfftps.lpre $z17, $z35, $z52, $z33         
        vcpfftps.hpre $z19, $z35, $z52, $z33      
        vcpfftps.lpre $z21, $z39, $z53, $z37       
        vcpfftps.hpre $z23, $z39, $z53, $z37

        vcpfftps.lpre $z24, $z42, $z54, $z40
        vcpfftps.hpre $z26, $z42, $z54, $z40
        vcpfftps.lpre $z28, $z46, $z55, $z44
        vcpfftps.hpre $z30, $z46, $z55, $z44
        vcpfftps.lpre $z25, $z43, $z54, $z41        
        vcpfftps.hpre $z27, $z43, $z54, $z41      
        vcpfftps.lpre $z29, $z47, $z55, $z45        
        vcpfftps.hpre $z31, $z47, $z55, $z45

        vcpfftps.l $z16, $z34, $z52, $z16
        vcpfftps.h $z18, $z34, $z52, $z18
        vcpfftps.l $z17, $z35, $z52, $z17       
        vcpfftps.h $z19, $z35, $z52, $z19      
        vcpfftps.l $z20, $z38, $z53, $z20
        vcpfftps.h $z22, $z38, $z53, $z22
        vcpfftps.l $z21, $z39, $z53, $z21      
        vcpfftps.h $z23, $z39, $z53, $z23

        vcpfftps.l $z24, $z42, $z54, $z24
        vcpfftps.h $z26, $z42, $z54, $z26
        vcpfftps.l $z25, $z43, $z54, $z25       
        vcpfftps.h $z27, $z43, $z54, $z27      
        vcpfftps.l $z28, $z46, $z55, $z28
        vcpfftps.h $z30, $z46, $z55, $z30
        vcpfftps.l $z29, $z47, $z55, $z29       
        vcpfftps.h $z31, $z47, $z55, $z31

fft_stage_10_g4:        
##B->B
        vcpfftps.lpre $z32, $z17, $z56, $z16
        vcpfftps.hpre $z33, $z17, $z56, $z16
        vcpfftps.lpre $z34, $z19, $z57, $z18        
        vcpfftps.hpre $z35, $z19, $z57, $z18      
        vcpfftps.lpre $z36, $z21, $z58, $z20
        vcpfftps.hpre $z37, $z21, $z58, $z20
        vcpfftps.lpre $z38, $z23, $z59, $z22      
        vcpfftps.hpre $z39, $z23, $z59, $z22

        vcpfftps.lpre $z40, $z25, $z60, $z24
        vcpfftps.hpre $z41, $z25, $z60, $z24
        vcpfftps.lpre $z42, $z27, $z61, $z26        
        vcpfftps.hpre $z43, $z27, $z61, $z26      
        vcpfftps.lpre $z44, $z29, $z62, $z28
        vcpfftps.hpre $z45, $z29, $z62, $z28
        vcpfftps.lpre $z46, $z31, $z63, $z30        
        vcpfftps.hpre $z47, $z31, $z63, $z30

        vcpfftps.l $z16, $z17, $z56, $z32
        vcpfftps.h $z17, $z17, $z56, $z33
        vcpfftps.l $z18, $z19, $z57, $z34       
        vcpfftps.h $z19, $z19, $z57, $z35       
        vcpfftps.l $z20, $z21, $z58, $z36
        vcpfftps.h $z21, $z21, $z58, $z37
        vcpfftps.l $z22, $z23, $z59, $z38       
        vcpfftps.h $z23, $z23, $z59, $z39

        vcpfftps.l $z24, $z25, $z60, $z40
        vcpfftps.h $z25, $z25, $z60, $z41
        vcpfftps.l $z26, $z27, $z61, $z42        
        vcpfftps.h $z27, $z27, $z61, $z43       
        vcpfftps.l $z28, $z29, $z62, $z44
        vcpfftps.h $z29, $z29, $z62, $z45
        vcpfftps.l $z30, $z31, $z63, $z46       
        vcpfftps.h $z31, $z31, $z63, $z47

        sd  t7, 0xb0(t0)
        sd  t7, 0xa0(t0)

        subu s0, s0, 0x1
        nop
        bnez s0, fft_cal_2_1
        nop

##seventh group
1:      bc2f(0, 0xffff)
        nop
        bc2f(2, 0xffff)
        nop
fft_stage_6_g7:        
##A->C
        vcpfftps.lpre $z32, $z8 , $z48, $z0 
        vcpfftps.lpre $z40, $z12, $z48, $z4 
        vcpfftps.lpre $z34, $z9 , $z48, $z1         
        vcpfftps.lpre $z42, $z13, $z48, $z5         
        vcpfftps.lpre $z36, $z10, $z48, $z2 
        vcpfftps.lpre $z44, $z14, $z48, $z6 
        vcpfftps.lpre $z38, $z11, $z48, $z3         
        vcpfftps.lpre $z46, $z15, $z48, $z7        

        vcpfftps.hpre $z33, $z8 , $z48, $z0 
        vcpfftps.hpre $z41, $z12, $z48, $z4 
        vcpfftps.hpre $z35, $z9 , $z48, $z1      
        vcpfftps.hpre $z43, $z13, $z48, $z5        
        vcpfftps.hpre $z37, $z10, $z48, $z2 
        vcpfftps.hpre $z45, $z14, $z48, $z6 
        vcpfftps.hpre $z39, $z11, $z48, $z3 
        vcpfftps.hpre $z47, $z15, $z48, $z7 
        
        vcpfftps.l $z32, $z8 , $z48, $z32
        vcpfftps.l $z40, $z12, $z48, $z40
        vcpfftps.l $z36, $z10, $z48, $z36
        vcpfftps.l $z44, $z14, $z48, $z44
        vcpfftps.l $z34, $z9 , $z48, $z34
        vcpfftps.l $z42, $z13, $z48, $z42
        vcpfftps.l $z38, $z11, $z48, $z38
        vcpfftps.l $z46, $z15, $z48, $z46
                                         
        vcpfftps.h $z33, $z8 , $z48, $z33
        vcpfftps.h $z41, $z12, $z48, $z41
        vcpfftps.h $z37, $z10, $z48, $z37
        vcpfftps.h $z45, $z14, $z48, $z45
        vcpfftps.h $z35, $z9 , $z48, $z35
        vcpfftps.h $z43, $z13, $z48, $z43
        vcpfftps.h $z39, $z11, $z48, $z39
        vcpfftps.h $z47, $z15, $z48, $z47
        
fft_stage_7_g7:        
##C->A
        vcpfftps.lpre $z0 , $z40, $z49, $z32
        vcpfftps.hpre $z8 , $z40, $z49, $z32
        vcpfftps.lpre $z4 , $z44, $z49, $z36
        vcpfftps.hpre $z12, $z44, $z49, $z36
        vcpfftps.lpre $z2 , $z42, $z49, $z34
        vcpfftps.hpre $z10, $z42, $z49, $z34
        vcpfftps.lpre $z6 , $z46, $z49, $z38
        vcpfftps.hpre $z14, $z46, $z49, $z38

        vcpfftps.lpre $z1 , $z41, $z49, $z33       
        vcpfftps.lpre $z5 , $z45, $z49, $z37       
        vcpfftps.hpre $z9 , $z41, $z49, $z33       
        vcpfftps.hpre $z13, $z45, $z49, $z37
        vcpfftps.lpre $z3 , $z43, $z49, $z35       
        vcpfftps.lpre $z7 , $z47, $z49, $z39       
        vcpfftps.hpre $z11, $z43, $z49, $z35 
        vcpfftps.hpre $z15, $z47, $z49, $z39 

        bc2f(6, 0xffff)
        nop
        sd  t7, 0x88(t0)
        nop
        
        vcpfftps.l $z0 , $z40, $z49, $z0 
        vcpfftps.h $z8 , $z40, $z49, $z8
        vcpfftps.l $z4 , $z44, $z49, $z4 
        vcpfftps.h $z12, $z44, $z49, $z12
        vcpfftps.l $z2 , $z42, $z49, $z2
        vcpfftps.l $z6 , $z46, $z49, $z6
        vcpfftps.l $z1 , $z41, $z49, $z1        
        vcpfftps.l $z5 , $z45, $z49, $z5       

        vcpfftps.h $z10, $z42, $z49, $z10
        vcpfftps.h $z14, $z46, $z49, $z14
        vcpfftps.l $z3 , $z43, $z49, $z3         
        vcpfftps.l $z7 , $z47, $z49, $z7       
        vcpfftps.h $z9 , $z41, $z49, $z9       
        vcpfftps.h $z13, $z45, $z49, $z13
        vcpfftps.h $z11, $z43, $z49, $z11
        vcpfftps.h $z15, $z47, $z49, $z15

fft_stage_8_g7:     
##A->C
        vcpfftps.lpre $z32, $z4 , $z50, $z0
        vcpfftps.hpre $z36, $z4 , $z50, $z0
        vcpfftps.lpre $z40, $z12, $z51, $z8 
        vcpfftps.hpre $z44, $z12, $z51, $z8
        vcpfftps.lpre $z34, $z6 , $z50, $z2
        vcpfftps.hpre $z38, $z6 , $z50, $z2
        vcpfftps.lpre $z33, $z5 , $z50, $z1        
        vcpfftps.hpre $z37, $z5 , $z50, $z1     

        vcpfftps.lpre $z35, $z7 , $z50, $z3         
        vcpfftps.hpre $z39, $z7 , $z50, $z3
        vcpfftps.lpre $z41, $z13, $z51, $z9         
        vcpfftps.hpre $z45, $z13, $z51, $z9 
        vcpfftps.lpre $z42, $z14, $z51, $z10
        vcpfftps.hpre $z46, $z14, $z51, $z10
        vcpfftps.lpre $z43, $z15, $z51, $z11       
        vcpfftps.hpre $z47, $z15, $z51, $z11

        vcpfftps.l $z32, $z4 , $z50, $z32
        vcpfftps.h $z36, $z4 , $z50, $z36
        vcpfftps.l $z34, $z6 , $z50, $z34
        vcpfftps.h $z38, $z6 , $z50, $z38
        vcpfftps.l $z33, $z5 , $z50, $z33      
        vcpfftps.h $z37, $z5 , $z50, $z37    
        vcpfftps.l $z35, $z7 , $z50, $z35    
        vcpfftps.h $z39, $z7 , $z50, $z39

        vcpfftps.l $z40, $z12, $z51, $z40
        vcpfftps.h $z44, $z12, $z51, $z44
        vcpfftps.l $z42, $z14, $z51, $z42
        vcpfftps.h $z46, $z14, $z51, $z46
        vcpfftps.l $z41, $z13, $z51, $z41      
        vcpfftps.h $z45, $z13, $z51, $z45
        vcpfftps.l $z43, $z15, $z51, $z43       
        vcpfftps.h $z47, $z15, $z51, $z47

        sd  t7, 0x90(t0)
        nop
        bc2f(4, 0xffff)
        nop

fft_stage_9_g7:              
##C->A
        vcpfftps.lpre $z0 , $z34, $z52, $z32
        vcpfftps.hpre $z2 , $z34, $z52, $z32
        vcpfftps.lpre $z4 , $z38, $z53, $z36
        vcpfftps.hpre $z6 , $z38, $z53, $z36
        vcpfftps.lpre $z1 , $z35, $z52, $z33         
        vcpfftps.hpre $z3 , $z35, $z52, $z33      
        vcpfftps.lpre $z5 , $z39, $z53, $z37      
        vcpfftps.hpre $z7 , $z39, $z53, $z37

        vcpfftps.lpre $z8 , $z42, $z54, $z40
        vcpfftps.hpre $z10, $z42, $z54, $z40
        vcpfftps.lpre $z12, $z46, $z55, $z44
        vcpfftps.hpre $z14, $z46, $z55, $z44
        vcpfftps.lpre $z9 , $z43, $z54, $z41        
        vcpfftps.hpre $z11, $z43, $z54, $z41      
        vcpfftps.lpre $z13, $z47, $z55, $z45        
        vcpfftps.hpre $z15, $z47, $z55, $z45

        vcpfftps.l $z0 , $z34, $z52, $z0 
        vcpfftps.h $z2 , $z34, $z52, $z2 
        vcpfftps.l $z1 , $z35, $z52, $z1         
        vcpfftps.h $z3 , $z35, $z52, $z3        
        vcpfftps.l $z4 , $z38, $z53, $z4 
        vcpfftps.h $z6 , $z38, $z53, $z6 
        vcpfftps.l $z5 , $z39, $z53, $z5       
        vcpfftps.h $z7 , $z39, $z53, $z7 

        vcpfftps.l $z8 , $z42, $z54, $z8 
        vcpfftps.h $z10, $z42, $z54, $z10
        vcpfftps.l $z9 , $z43, $z54, $z9         
        vcpfftps.h $z11, $z43, $z54, $z11       
        vcpfftps.l $z12, $z46, $z55, $z12
        vcpfftps.h $z14, $z46, $z55, $z14
        vcpfftps.l $z13, $z47, $z55, $z13        
        vcpfftps.h $z15, $z47, $z55, $z15

fft_stage_10_g7:        
##A->A
        vcpfftps.lpre $z32, $z1 , $z56, $z0 
        vcpfftps.hpre $z33, $z1 , $z56, $z0
        vcpfftps.lpre $z34, $z3 , $z57, $z2         
        vcpfftps.hpre $z35, $z3 , $z57, $z2        
        vcpfftps.lpre $z36, $z5 , $z58, $z4 
        vcpfftps.hpre $z37, $z5 , $z58, $z4 
        vcpfftps.lpre $z38, $z7 , $z59, $z6        
        vcpfftps.hpre $z39, $z7 , $z59, $z6

        vcpfftps.lpre $z40, $z9 , $z60, $z8 
        vcpfftps.hpre $z41, $z9 , $z60, $z8 
        vcpfftps.lpre $z42, $z11, $z61, $z10        
        vcpfftps.hpre $z43, $z11, $z61, $z10       
        vcpfftps.lpre $z44, $z13, $z62, $z12
        vcpfftps.hpre $z45, $z13, $z62, $z12
        vcpfftps.lpre $z46, $z15, $z63, $z14        
        vcpfftps.hpre $z47, $z15, $z63, $z14

        vcpfftps.l $z0 , $z1 , $z56, $z32
        vcpfftps.h $z1 , $z1 , $z56, $z33
        vcpfftps.l $z2 , $z3 , $z57, $z34       
        vcpfftps.h $z3 , $z3 , $z57, $z35       
        vcpfftps.l $z4 , $z5 , $z58, $z36
        vcpfftps.h $z5 , $z5 , $z58, $z37
        vcpfftps.l $z6 , $z7 , $z59, $z38      
        vcpfftps.h $z7 , $z7 , $z59, $z39

        vcpfftps.l $z8 , $z9 , $z60, $z40
        vcpfftps.h $z9 , $z9 , $z60, $z41
        vcpfftps.l $z10, $z11, $z61, $z42        
        vcpfftps.h $z11, $z11, $z61, $z43       
        vcpfftps.l $z12, $z13, $z62, $z44
        vcpfftps.h $z13, $z13, $z62, $z45
        vcpfftps.l $z14, $z15, $z63, $z46       
        vcpfftps.h $z15, $z15, $z63, $z47

        sd  t7, 0xb0(t0)
        sd  t7, 0xa0(t0)
       
##eighth group
1:      bc2f(1, 0xffff)
        nop
        bc2f(2, 0xffff)
        nop
fft_stage_6_g8:        
##B->C
        vcpfftps.lpre $z32, $z24, $z48, $z16
        vcpfftps.lpre $z40, $z28, $z48, $z20
        vcpfftps.lpre $z34, $z25, $z48, $z17        
        vcpfftps.lpre $z42, $z29, $z48, $z21        
        vcpfftps.lpre $z36, $z26, $z48, $z18
        vcpfftps.lpre $z44, $z30, $z48, $z22
        vcpfftps.lpre $z38, $z27, $z48, $z19        
        vcpfftps.lpre $z46, $z31, $z48, $z23       

        vcpfftps.hpre $z33, $z24, $z48, $z16
        vcpfftps.hpre $z41, $z28, $z48, $z20
        vcpfftps.hpre $z35, $z25, $z48, $z17    
        vcpfftps.hpre $z43, $z29, $z48, $z21      
        vcpfftps.hpre $z37, $z26, $z48, $z18
        vcpfftps.hpre $z45, $z30, $z48, $z22
        vcpfftps.hpre $z39, $z27, $z48, $z19
        vcpfftps.hpre $z47, $z31, $z48, $z23
        
        vcpfftps.l $z32, $z24, $z48, $z32
        vcpfftps.l $z40, $z28, $z48, $z40
        vcpfftps.l $z36, $z26, $z48, $z36
        vcpfftps.l $z44, $z30, $z48, $z44
        vcpfftps.l $z34, $z25, $z48, $z34
        vcpfftps.l $z42, $z29, $z48, $z42
        vcpfftps.l $z38, $z27, $z48, $z38
        vcpfftps.l $z46, $z31, $z48, $z46
                                         
        vcpfftps.h $z33, $z24, $z48, $z33
        vcpfftps.h $z41, $z28, $z48, $z41
        vcpfftps.h $z37, $z26, $z48, $z37
        vcpfftps.h $z45, $z30, $z48, $z45
        vcpfftps.h $z35, $z25, $z48, $z35
        vcpfftps.h $z43, $z29, $z48, $z43
        vcpfftps.h $z39, $z27, $z48, $z39
        vcpfftps.h $z47, $z31, $z48, $z47
        
fft_stage_7_g8:        
##C->B
        vcpfftps.lpre $z16, $z40, $z49, $z32
        vcpfftps.hpre $z24, $z40, $z49, $z32
        vcpfftps.lpre $z20, $z44, $z49, $z36
        vcpfftps.hpre $z28, $z44, $z49, $z36
        vcpfftps.lpre $z18, $z42, $z49, $z34
        vcpfftps.hpre $z26, $z42, $z49, $z34
        vcpfftps.lpre $z22, $z46, $z49, $z38
        vcpfftps.hpre $z30, $z46, $z49, $z38

        vcpfftps.lpre $z17, $z41, $z49, $z33       
        vcpfftps.lpre $z21, $z45, $z49, $z37       
        vcpfftps.hpre $z25, $z41, $z49, $z33       
        vcpfftps.hpre $z29, $z45, $z49, $z37
        vcpfftps.lpre $z19, $z43, $z49, $z35       
        vcpfftps.lpre $z23, $z47, $z49, $z39       
        vcpfftps.hpre $z27, $z43, $z49, $z35
        vcpfftps.hpre $z31, $z47, $z49, $z39 

        bc2f(6, 0xffff)
        nop
        sd  t7, 0x80(t0)
        sd  s3, 0x0(t0)
        
        vcpfftps.l $z16, $z40, $z49, $z16
        vcpfftps.h $z24, $z40, $z49, $z24
        vcpfftps.l $z20, $z44, $z49, $z20
        vcpfftps.h $z28, $z44, $z49, $z28
        vcpfftps.l $z18, $z42, $z49, $z18
        vcpfftps.l $z22, $z46, $z49, $z22
        vcpfftps.l $z17, $z41, $z49, $z17       
        vcpfftps.l $z21, $z45, $z49, $z21    

        vcpfftps.h $z26, $z42, $z49, $z26
        vcpfftps.h $z30, $z46, $z49, $z30
        vcpfftps.l $z19, $z43, $z49, $z19       
        vcpfftps.l $z23, $z47, $z49, $z23     
        vcpfftps.h $z25, $z41, $z49, $z25     
        vcpfftps.h $z29, $z45, $z49, $z29
        vcpfftps.h $z27, $z43, $z49, $z27
        vcpfftps.h $z31, $z47, $z49, $z31

fft_stage_8_g8:     
##B->C
        vcpfftps.lpre $z32, $z20, $z50, $z16
        vcpfftps.hpre $z36, $z20, $z50, $z16
        vcpfftps.lpre $z40, $z28, $z51, $z24
        vcpfftps.hpre $z44, $z28, $z51, $z24
        vcpfftps.lpre $z34, $z22, $z50, $z18
        vcpfftps.hpre $z38, $z22, $z50, $z18
        vcpfftps.lpre $z33, $z21, $z50, $z17       
        vcpfftps.hpre $z37, $z21, $z50, $z17    

        vcpfftps.lpre $z35, $z23, $z50, $z19        
        vcpfftps.hpre $z39, $z23, $z50, $z19
        vcpfftps.lpre $z41, $z29, $z51, $z25        
        vcpfftps.hpre $z45, $z29, $z51, $z25
        vcpfftps.lpre $z42, $z30, $z51, $z26
        vcpfftps.hpre $z46, $z30, $z51, $z26
        vcpfftps.lpre $z43, $z31, $z51, $z27       
        vcpfftps.hpre $z47, $z31, $z51, $z27

        vcpfftps.l $z32, $z20, $z50, $z32
        vcpfftps.h $z36, $z20, $z50, $z36
        vcpfftps.l $z34, $z22, $z50, $z34
        vcpfftps.h $z38, $z22, $z50, $z38
        vcpfftps.l $z33, $z21, $z50, $z33      
        vcpfftps.h $z37, $z21, $z50, $z37   
        vcpfftps.l $z35, $z23, $z50, $z35     
        vcpfftps.h $z39, $z23, $z50, $z39

        vcpfftps.l $z40, $z28, $z51, $z40
        vcpfftps.h $z44, $z28, $z51, $z44
        vcpfftps.l $z42, $z30, $z51, $z42
        vcpfftps.h $z46, $z30, $z51, $z46
        vcpfftps.l $z41, $z29, $z51, $z41      
        vcpfftps.h $z45, $z29, $z51, $z45
        vcpfftps.l $z43, $z31, $z51, $z43       
        vcpfftps.h $z47, $z31, $z51, $z47

        sd  t7, 0x90(t0)
        nop
        bc2f(4, 0xffff)
        nop

fft_stage_9_g8:              
##C->A
        vcpfftps.lpre $z16, $z34, $z52, $z32
        vcpfftps.hpre $z18, $z34, $z52, $z32
        vcpfftps.lpre $z20, $z38, $z53, $z36
        vcpfftps.hpre $z22, $z38, $z53, $z36
        vcpfftps.lpre $z17, $z35, $z52, $z33         
        vcpfftps.hpre $z19, $z35, $z52, $z33      
        vcpfftps.lpre $z21, $z39, $z53, $z37       
        vcpfftps.hpre $z23, $z39, $z53, $z37

        vcpfftps.lpre $z24, $z42, $z54, $z40
        vcpfftps.hpre $z26, $z42, $z54, $z40
        vcpfftps.lpre $z28, $z46, $z55, $z44
        vcpfftps.hpre $z30, $z46, $z55, $z44
        vcpfftps.lpre $z25, $z43, $z54, $z41        
        vcpfftps.hpre $z27, $z43, $z54, $z41      
        vcpfftps.lpre $z29, $z47, $z55, $z45        
        vcpfftps.hpre $z31, $z47, $z55, $z45

        vcpfftps.l $z16, $z34, $z52, $z16
        vcpfftps.h $z18, $z34, $z52, $z18
        vcpfftps.l $z17, $z35, $z52, $z17       
        vcpfftps.h $z19, $z35, $z52, $z19      
        vcpfftps.l $z20, $z38, $z53, $z20
        vcpfftps.h $z22, $z38, $z53, $z22
        vcpfftps.l $z21, $z39, $z53, $z21      
        vcpfftps.h $z23, $z39, $z53, $z23

        vcpfftps.l $z24, $z42, $z54, $z24
        vcpfftps.h $z26, $z42, $z54, $z26
        vcpfftps.l $z25, $z43, $z54, $z25       
        vcpfftps.h $z27, $z43, $z54, $z27      
        vcpfftps.l $z28, $z46, $z55, $z28
        vcpfftps.h $z30, $z46, $z55, $z30
        vcpfftps.l $z29, $z47, $z55, $z29       
        vcpfftps.h $z31, $z47, $z55, $z31

fft_stage_10_g8:        
##B->B
        vcpfftps.lpre $z32, $z17, $z56, $z16
        vcpfftps.hpre $z33, $z17, $z56, $z16
        vcpfftps.lpre $z34, $z19, $z57, $z18        
        vcpfftps.hpre $z35, $z19, $z57, $z18      
        vcpfftps.lpre $z36, $z21, $z58, $z20
        vcpfftps.hpre $z37, $z21, $z58, $z20
        vcpfftps.lpre $z38, $z23, $z59, $z22      
        vcpfftps.hpre $z39, $z23, $z59, $z22

        vcpfftps.lpre $z40, $z25, $z60, $z24
        vcpfftps.hpre $z41, $z25, $z60, $z24
        vcpfftps.lpre $z42, $z27, $z61, $z26        
        vcpfftps.hpre $z43, $z27, $z61, $z26      
        vcpfftps.lpre $z44, $z29, $z62, $z28
        vcpfftps.hpre $z45, $z29, $z62, $z28
        vcpfftps.lpre $z46, $z31, $z63, $z30        
        vcpfftps.hpre $z47, $z31, $z63, $z30

        vcpfftps.l $z16, $z17, $z56, $z32
        vcpfftps.h $z17, $z17, $z56, $z33
        vcpfftps.l $z18, $z19, $z57, $z34       
        vcpfftps.h $z19, $z19, $z57, $z35       
        vcpfftps.l $z20, $z21, $z58, $z36
        vcpfftps.h $z21, $z21, $z58, $z37
        vcpfftps.l $z22, $z23, $z59, $z38       
        vcpfftps.h $z23, $z23, $z59, $z39

        vcpfftps.l $z24, $z25, $z60, $z40
        vcpfftps.h $z25, $z25, $z60, $z41
        vcpfftps.l $z26, $z27, $z61, $z42        
        vcpfftps.h $z27, $z27, $z61, $z43       
        vcpfftps.l $z28, $z29, $z62, $z44
        vcpfftps.h $z29, $z29, $z62, $z45
        vcpfftps.l $z30, $z31, $z63, $z46       
        vcpfftps.h $z31, $z31, $z63, $z47

        sd  t7, 0xb0(t0)
        sd  t7, 0xa0(t0)

        li  s0, 0x4
        nop
        
fft_cal_2_2:
##fifth group
1:      bc2f(0, 0xffff)
        nop
        bc2f(2, 0xffff)
        nop
fft_stage_6_g9:        
##A->C
        vcpfftps.lpre $z32, $z8 , $z48, $z0 
        vcpfftps.lpre $z40, $z12, $z48, $z4 
        vcpfftps.lpre $z34, $z9 , $z48, $z1         
        vcpfftps.lpre $z42, $z13, $z48, $z5         
        vcpfftps.lpre $z36, $z10, $z48, $z2 
        vcpfftps.lpre $z44, $z14, $z48, $z6 
        vcpfftps.lpre $z38, $z11, $z48, $z3         
        vcpfftps.lpre $z46, $z15, $z48, $z7        

        vcpfftps.hpre $z33, $z8 , $z48, $z0 
        vcpfftps.hpre $z41, $z12, $z48, $z4 
        vcpfftps.hpre $z35, $z9 , $z48, $z1      
        vcpfftps.hpre $z43, $z13, $z48, $z5        
        vcpfftps.hpre $z37, $z10, $z48, $z2 
        vcpfftps.hpre $z45, $z14, $z48, $z6 
        vcpfftps.hpre $z39, $z11, $z48, $z3 
        vcpfftps.hpre $z47, $z15, $z48, $z7 
        
        vcpfftps.l $z32, $z8 , $z48, $z32
        vcpfftps.l $z40, $z12, $z48, $z40
        vcpfftps.l $z36, $z10, $z48, $z36
        vcpfftps.l $z44, $z14, $z48, $z44
        vcpfftps.l $z34, $z9 , $z48, $z34
        vcpfftps.l $z42, $z13, $z48, $z42
        vcpfftps.l $z38, $z11, $z48, $z38
        vcpfftps.l $z46, $z15, $z48, $z46
                                         
        vcpfftps.h $z33, $z8 , $z48, $z33
        vcpfftps.h $z41, $z12, $z48, $z41
        vcpfftps.h $z37, $z10, $z48, $z37
        vcpfftps.h $z45, $z14, $z48, $z45
        vcpfftps.h $z35, $z9 , $z48, $z35
        vcpfftps.h $z43, $z13, $z48, $z43
        vcpfftps.h $z39, $z11, $z48, $z39
        vcpfftps.h $z47, $z15, $z48, $z47
        
fft_stage_7_g9:        
##C->A
        vcpfftps.lpre $z0 , $z40, $z49, $z32
        vcpfftps.hpre $z8 , $z40, $z49, $z32
        vcpfftps.lpre $z4 , $z44, $z49, $z36
        vcpfftps.hpre $z12, $z44, $z49, $z36
        vcpfftps.lpre $z2 , $z42, $z49, $z34
        vcpfftps.hpre $z10, $z42, $z49, $z34
        vcpfftps.lpre $z6 , $z46, $z49, $z38
        vcpfftps.hpre $z14, $z46, $z49, $z38

        vcpfftps.lpre $z1 , $z41, $z49, $z33       
        vcpfftps.lpre $z5 , $z45, $z49, $z37       
        vcpfftps.hpre $z9 , $z41, $z49, $z33       
        vcpfftps.hpre $z13, $z45, $z49, $z37
        vcpfftps.lpre $z3 , $z43, $z49, $z35       
        vcpfftps.lpre $z7 , $z47, $z49, $z39       
        vcpfftps.hpre $z11, $z43, $z49, $z35 
        vcpfftps.hpre $z15, $z47, $z49, $z39 

        bc2f(6, 0xffff)
        nop
        sd  t7, 0x88(t0)
        nop
        
        vcpfftps.l $z0 , $z40, $z49, $z0 
        vcpfftps.h $z8 , $z40, $z49, $z8
        vcpfftps.l $z4 , $z44, $z49, $z4 
        vcpfftps.h $z12, $z44, $z49, $z12
        vcpfftps.l $z2 , $z42, $z49, $z2
        vcpfftps.l $z6 , $z46, $z49, $z6
        vcpfftps.l $z1 , $z41, $z49, $z1        
        vcpfftps.l $z5 , $z45, $z49, $z5       

        vcpfftps.h $z10, $z42, $z49, $z10
        vcpfftps.h $z14, $z46, $z49, $z14
        vcpfftps.l $z3 , $z43, $z49, $z3         
        vcpfftps.l $z7 , $z47, $z49, $z7       
        vcpfftps.h $z9 , $z41, $z49, $z9       
        vcpfftps.h $z13, $z45, $z49, $z13
        vcpfftps.h $z11, $z43, $z49, $z11
        vcpfftps.h $z15, $z47, $z49, $z15

fft_stage_8_g9:     
##A->C
        vcpfftps.lpre $z32, $z4 , $z50, $z0
        vcpfftps.hpre $z36, $z4 , $z50, $z0
        vcpfftps.lpre $z40, $z12, $z51, $z8 
        vcpfftps.hpre $z44, $z12, $z51, $z8
        vcpfftps.lpre $z34, $z6 , $z50, $z2
        vcpfftps.hpre $z38, $z6 , $z50, $z2
        vcpfftps.lpre $z33, $z5 , $z50, $z1        
        vcpfftps.hpre $z37, $z5 , $z50, $z1     

        vcpfftps.lpre $z35, $z7 , $z50, $z3         
        vcpfftps.hpre $z39, $z7 , $z50, $z3
        vcpfftps.lpre $z41, $z13, $z51, $z9         
        vcpfftps.hpre $z45, $z13, $z51, $z9 
        vcpfftps.lpre $z42, $z14, $z51, $z10
        vcpfftps.hpre $z46, $z14, $z51, $z10
        vcpfftps.lpre $z43, $z15, $z51, $z11       
        vcpfftps.hpre $z47, $z15, $z51, $z11

        vcpfftps.l $z32, $z4 , $z50, $z32
        vcpfftps.h $z36, $z4 , $z50, $z36
        vcpfftps.l $z34, $z6 , $z50, $z34
        vcpfftps.h $z38, $z6 , $z50, $z38
        vcpfftps.l $z33, $z5 , $z50, $z33      
        vcpfftps.h $z37, $z5 , $z50, $z37    
        vcpfftps.l $z35, $z7 , $z50, $z35    
        vcpfftps.h $z39, $z7 , $z50, $z39

        vcpfftps.l $z40, $z12, $z51, $z40
        vcpfftps.h $z44, $z12, $z51, $z44
        vcpfftps.l $z42, $z14, $z51, $z42
        vcpfftps.h $z46, $z14, $z51, $z46
        vcpfftps.l $z41, $z13, $z51, $z41      
        vcpfftps.h $z45, $z13, $z51, $z45
        vcpfftps.l $z43, $z15, $z51, $z43       
        vcpfftps.h $z47, $z15, $z51, $z47

        sd  t7, 0x90(t0)
        nop
        bc2f(4, 0xffff)
        nop

fft_stage_9_g9:              
##C->A
        vcpfftps.lpre $z0 , $z34, $z52, $z32
        vcpfftps.hpre $z2 , $z34, $z52, $z32
        vcpfftps.lpre $z4 , $z38, $z53, $z36
        vcpfftps.hpre $z6 , $z38, $z53, $z36
        vcpfftps.lpre $z1 , $z35, $z52, $z33         
        vcpfftps.hpre $z3 , $z35, $z52, $z33      
        vcpfftps.lpre $z5 , $z39, $z53, $z37      
        vcpfftps.hpre $z7 , $z39, $z53, $z37

        vcpfftps.lpre $z8 , $z42, $z54, $z40
        vcpfftps.hpre $z10, $z42, $z54, $z40
        vcpfftps.lpre $z12, $z46, $z55, $z44
        vcpfftps.hpre $z14, $z46, $z55, $z44
        vcpfftps.lpre $z9 , $z43, $z54, $z41        
        vcpfftps.hpre $z11, $z43, $z54, $z41      
        vcpfftps.lpre $z13, $z47, $z55, $z45        
        vcpfftps.hpre $z15, $z47, $z55, $z45

        vcpfftps.l $z0 , $z34, $z52, $z0 
        vcpfftps.h $z2 , $z34, $z52, $z2 
        vcpfftps.l $z1 , $z35, $z52, $z1         
        vcpfftps.h $z3 , $z35, $z52, $z3        
        vcpfftps.l $z4 , $z38, $z53, $z4 
        vcpfftps.h $z6 , $z38, $z53, $z6 
        vcpfftps.l $z5 , $z39, $z53, $z5       
        vcpfftps.h $z7 , $z39, $z53, $z7 

        vcpfftps.l $z8 , $z42, $z54, $z8 
        vcpfftps.h $z10, $z42, $z54, $z10
        vcpfftps.l $z9 , $z43, $z54, $z9         
        vcpfftps.h $z11, $z43, $z54, $z11       
        vcpfftps.l $z12, $z46, $z55, $z12
        vcpfftps.h $z14, $z46, $z55, $z14
        vcpfftps.l $z13, $z47, $z55, $z13        
        vcpfftps.h $z15, $z47, $z55, $z15

fft_stage_10_g9:        
##A->A
        vcpfftps.lpre $z32, $z1 , $z56, $z0 
        vcpfftps.hpre $z33, $z1 , $z56, $z0
        vcpfftps.lpre $z34, $z3 , $z57, $z2         
        vcpfftps.hpre $z35, $z3 , $z57, $z2        
        vcpfftps.lpre $z36, $z5 , $z58, $z4 
        vcpfftps.hpre $z37, $z5 , $z58, $z4 
        vcpfftps.lpre $z38, $z7 , $z59, $z6        
        vcpfftps.hpre $z39, $z7 , $z59, $z6

        vcpfftps.lpre $z40, $z9 , $z60, $z8 
        vcpfftps.hpre $z41, $z9 , $z60, $z8 
        vcpfftps.lpre $z42, $z11, $z61, $z10        
        vcpfftps.hpre $z43, $z11, $z61, $z10       
        vcpfftps.lpre $z44, $z13, $z62, $z12
        vcpfftps.hpre $z45, $z13, $z62, $z12
        vcpfftps.lpre $z46, $z15, $z63, $z14        
        vcpfftps.hpre $z47, $z15, $z63, $z14

        vcpfftps.l $z0 , $z1 , $z56, $z32
        vcpfftps.h $z1 , $z1 , $z56, $z33
        vcpfftps.l $z2 , $z3 , $z57, $z34       
        vcpfftps.h $z3 , $z3 , $z57, $z35       
        vcpfftps.l $z4 , $z5 , $z58, $z36
        vcpfftps.h $z5 , $z5 , $z58, $z37
        vcpfftps.l $z6 , $z7 , $z59, $z38      
        vcpfftps.h $z7 , $z7 , $z59, $z39

        vcpfftps.l $z8 , $z9 , $z60, $z40
        vcpfftps.h $z9 , $z9 , $z60, $z41
        vcpfftps.l $z10, $z11, $z61, $z42        
        vcpfftps.h $z11, $z11, $z61, $z43       
        vcpfftps.l $z12, $z13, $z62, $z44
        vcpfftps.h $z13, $z13, $z62, $z45
        vcpfftps.l $z14, $z15, $z63, $z46       
        vcpfftps.h $z15, $z15, $z63, $z47

        sd  t7, 0xb0(t0)
        sd  t7, 0xa0(t0)
       
##fourth group
1:      bc2f(1, 0xffff)
        nop
        bc2f(2, 0xffff)
        nop
fft_stage_6_g10:       
##B->C
        vcpfftps.lpre $z32, $z24, $z48, $z16
        vcpfftps.lpre $z40, $z28, $z48, $z20
        vcpfftps.lpre $z34, $z25, $z48, $z17        
        vcpfftps.lpre $z42, $z29, $z48, $z21        
        vcpfftps.lpre $z36, $z26, $z48, $z18
        vcpfftps.lpre $z44, $z30, $z48, $z22
        vcpfftps.lpre $z38, $z27, $z48, $z19        
        vcpfftps.lpre $z46, $z31, $z48, $z23       

        vcpfftps.hpre $z33, $z24, $z48, $z16
        vcpfftps.hpre $z41, $z28, $z48, $z20
        vcpfftps.hpre $z35, $z25, $z48, $z17    
        vcpfftps.hpre $z43, $z29, $z48, $z21      
        vcpfftps.hpre $z37, $z26, $z48, $z18
        vcpfftps.hpre $z45, $z30, $z48, $z22
        vcpfftps.hpre $z39, $z27, $z48, $z19
        vcpfftps.hpre $z47, $z31, $z48, $z23
        
        vcpfftps.l $z32, $z24, $z48, $z32
        vcpfftps.l $z40, $z28, $z48, $z40
        vcpfftps.l $z36, $z26, $z48, $z36
        vcpfftps.l $z44, $z30, $z48, $z44
        vcpfftps.l $z34, $z25, $z48, $z34
        vcpfftps.l $z42, $z29, $z48, $z42
        vcpfftps.l $z38, $z27, $z48, $z38
        vcpfftps.l $z46, $z31, $z48, $z46
                                         
        vcpfftps.h $z33, $z24, $z48, $z33
        vcpfftps.h $z41, $z28, $z48, $z41
        vcpfftps.h $z37, $z26, $z48, $z37
        vcpfftps.h $z45, $z30, $z48, $z45
        vcpfftps.h $z35, $z25, $z48, $z35
        vcpfftps.h $z43, $z29, $z48, $z43
        vcpfftps.h $z39, $z27, $z48, $z39
        vcpfftps.h $z47, $z31, $z48, $z47
        
fft_stage_7_g10:       
##C->B
        vcpfftps.lpre $z16, $z40, $z49, $z32
        vcpfftps.hpre $z24, $z40, $z49, $z32
        vcpfftps.lpre $z20, $z44, $z49, $z36
        vcpfftps.hpre $z28, $z44, $z49, $z36
        vcpfftps.lpre $z18, $z42, $z49, $z34
        vcpfftps.hpre $z26, $z42, $z49, $z34
        vcpfftps.lpre $z22, $z46, $z49, $z38
        vcpfftps.hpre $z30, $z46, $z49, $z38

        vcpfftps.lpre $z17, $z41, $z49, $z33       
        vcpfftps.lpre $z21, $z45, $z49, $z37       
        vcpfftps.hpre $z25, $z41, $z49, $z33       
        vcpfftps.hpre $z29, $z45, $z49, $z37
        vcpfftps.lpre $z19, $z43, $z49, $z35       
        vcpfftps.lpre $z23, $z47, $z49, $z39       
        vcpfftps.hpre $z27, $z43, $z49, $z35
        vcpfftps.hpre $z31, $z47, $z49, $z39 

        bc2f(6, 0xffff)
        nop
        sd  t7, 0x80(t0)
        nop
        
        vcpfftps.l $z16, $z40, $z49, $z16
        vcpfftps.h $z24, $z40, $z49, $z24
        vcpfftps.l $z20, $z44, $z49, $z20
        vcpfftps.h $z28, $z44, $z49, $z28
        vcpfftps.l $z18, $z42, $z49, $z18
        vcpfftps.l $z22, $z46, $z49, $z22
        vcpfftps.l $z17, $z41, $z49, $z17       
        vcpfftps.l $z21, $z45, $z49, $z21    

        vcpfftps.h $z26, $z42, $z49, $z26
        vcpfftps.h $z30, $z46, $z49, $z30
        vcpfftps.l $z19, $z43, $z49, $z19       
        vcpfftps.l $z23, $z47, $z49, $z23     
        vcpfftps.h $z25, $z41, $z49, $z25     
        vcpfftps.h $z29, $z45, $z49, $z29
        vcpfftps.h $z27, $z43, $z49, $z27
        vcpfftps.h $z31, $z47, $z49, $z31

fft_stage_8_g10:    
##B->C
        vcpfftps.lpre $z32, $z20, $z50, $z16
        vcpfftps.hpre $z36, $z20, $z50, $z16
        vcpfftps.lpre $z40, $z28, $z51, $z24
        vcpfftps.hpre $z44, $z28, $z51, $z24
        vcpfftps.lpre $z34, $z22, $z50, $z18
        vcpfftps.hpre $z38, $z22, $z50, $z18
        vcpfftps.lpre $z33, $z21, $z50, $z17       
        vcpfftps.hpre $z37, $z21, $z50, $z17    

        vcpfftps.lpre $z35, $z23, $z50, $z19        
        vcpfftps.hpre $z39, $z23, $z50, $z19
        vcpfftps.lpre $z41, $z29, $z51, $z25        
        vcpfftps.hpre $z45, $z29, $z51, $z25
        vcpfftps.lpre $z42, $z30, $z51, $z26
        vcpfftps.hpre $z46, $z30, $z51, $z26
        vcpfftps.lpre $z43, $z31, $z51, $z27       
        vcpfftps.hpre $z47, $z31, $z51, $z27

        vcpfftps.l $z32, $z20, $z50, $z32
        vcpfftps.h $z36, $z20, $z50, $z36
        vcpfftps.l $z34, $z22, $z50, $z34
        vcpfftps.h $z38, $z22, $z50, $z38
        vcpfftps.l $z33, $z21, $z50, $z33      
        vcpfftps.h $z37, $z21, $z50, $z37   
        vcpfftps.l $z35, $z23, $z50, $z35     
        vcpfftps.h $z39, $z23, $z50, $z39

        vcpfftps.l $z40, $z28, $z51, $z40
        vcpfftps.h $z44, $z28, $z51, $z44
        vcpfftps.l $z42, $z30, $z51, $z42
        vcpfftps.h $z46, $z30, $z51, $z46
        vcpfftps.l $z41, $z29, $z51, $z41      
        vcpfftps.h $z45, $z29, $z51, $z45
        vcpfftps.l $z43, $z31, $z51, $z43       
        vcpfftps.h $z47, $z31, $z51, $z47

        sd  t7, 0x90(t0)
        nop
        bc2f(4, 0xffff)
        nop

fft_stage_9_g10:             
##C->A
        vcpfftps.lpre $z16, $z34, $z52, $z32
        vcpfftps.hpre $z18, $z34, $z52, $z32
        vcpfftps.lpre $z20, $z38, $z53, $z36
        vcpfftps.hpre $z22, $z38, $z53, $z36
        vcpfftps.lpre $z17, $z35, $z52, $z33         
        vcpfftps.hpre $z19, $z35, $z52, $z33      
        vcpfftps.lpre $z21, $z39, $z53, $z37       
        vcpfftps.hpre $z23, $z39, $z53, $z37

        vcpfftps.lpre $z24, $z42, $z54, $z40
        vcpfftps.hpre $z26, $z42, $z54, $z40
        vcpfftps.lpre $z28, $z46, $z55, $z44
        vcpfftps.hpre $z30, $z46, $z55, $z44
        vcpfftps.lpre $z25, $z43, $z54, $z41        
        vcpfftps.hpre $z27, $z43, $z54, $z41      
        vcpfftps.lpre $z29, $z47, $z55, $z45        
        vcpfftps.hpre $z31, $z47, $z55, $z45

        vcpfftps.l $z16, $z34, $z52, $z16
        vcpfftps.h $z18, $z34, $z52, $z18
        vcpfftps.l $z17, $z35, $z52, $z17       
        vcpfftps.h $z19, $z35, $z52, $z19      
        vcpfftps.l $z20, $z38, $z53, $z20
        vcpfftps.h $z22, $z38, $z53, $z22
        vcpfftps.l $z21, $z39, $z53, $z21      
        vcpfftps.h $z23, $z39, $z53, $z23

        vcpfftps.l $z24, $z42, $z54, $z24
        vcpfftps.h $z26, $z42, $z54, $z26
        vcpfftps.l $z25, $z43, $z54, $z25       
        vcpfftps.h $z27, $z43, $z54, $z27      
        vcpfftps.l $z28, $z46, $z55, $z28
        vcpfftps.h $z30, $z46, $z55, $z30
        vcpfftps.l $z29, $z47, $z55, $z29       
        vcpfftps.h $z31, $z47, $z55, $z31

fft_stage_10_g10:       
##B->B
        vcpfftps.lpre $z32, $z17, $z56, $z16
        vcpfftps.hpre $z33, $z17, $z56, $z16
        vcpfftps.lpre $z34, $z19, $z57, $z18        
        vcpfftps.hpre $z35, $z19, $z57, $z18      
        vcpfftps.lpre $z36, $z21, $z58, $z20
        vcpfftps.hpre $z37, $z21, $z58, $z20
        vcpfftps.lpre $z38, $z23, $z59, $z22      
        vcpfftps.hpre $z39, $z23, $z59, $z22

        vcpfftps.lpre $z40, $z25, $z60, $z24
        vcpfftps.hpre $z41, $z25, $z60, $z24
        vcpfftps.lpre $z42, $z27, $z61, $z26        
        vcpfftps.hpre $z43, $z27, $z61, $z26      
        vcpfftps.lpre $z44, $z29, $z62, $z28
        vcpfftps.hpre $z45, $z29, $z62, $z28
        vcpfftps.lpre $z46, $z31, $z63, $z30        
        vcpfftps.hpre $z47, $z31, $z63, $z30

        vcpfftps.l $z16, $z17, $z56, $z32
        vcpfftps.h $z17, $z17, $z56, $z33
        vcpfftps.l $z18, $z19, $z57, $z34       
        vcpfftps.h $z19, $z19, $z57, $z35       
        vcpfftps.l $z20, $z21, $z58, $z36
        vcpfftps.h $z21, $z21, $z58, $z37
        vcpfftps.l $z22, $z23, $z59, $z38       
        vcpfftps.h $z23, $z23, $z59, $z39

        vcpfftps.l $z24, $z25, $z60, $z40
        vcpfftps.h $z25, $z25, $z60, $z41
        vcpfftps.l $z26, $z27, $z61, $z42        
        vcpfftps.h $z27, $z27, $z61, $z43       
        vcpfftps.l $z28, $z29, $z62, $z44
        vcpfftps.h $z29, $z29, $z62, $z45
        vcpfftps.l $z30, $z31, $z63, $z46       
        vcpfftps.h $z31, $z31, $z63, $z47

        sd  t7, 0xb0(t0)
        sd  t7, 0xa0(t0)

        subu s0, s0, 0x1
        nop
        bnez s0, fft_cal_2_2
        nop

##        OUT_PERFCNT0("fft cycles:")

1:      ld 	s0, 0x00(sp)
	    ld	s1, 0x08(sp)
	    ld	s2, 0x10(sp)
	    ld	s3, 0x18(sp)
	    ld	s4, 0x20(sp)
	    ld	s5, 0x28(sp)
    	ld	s6, 0x30(sp)
	    ld	s7, 0x38(sp)
        ld      ra, 0x40(sp)
        jr      ra
        daddiu  sp, sp, 256
        .end fft_1kp 


##############################################################################
        .rdata
        .align  5
fft_1kp_begin:
        .asciiz "fft 1kp  test begin.\n"
##############################################################################
        .rdata		
        .align 5	

fft_din:
.float		  -36.95914705 ,   -45.97945322	##group 1
.float		  -67.07239627 ,  -106.33111995
.float		   15.40703176 ,   -44.89177903
.float		  -48.78902559 ,   109.61559869
.float		  -21.97161306 ,   -52.90669471
.float		  -97.97752639 ,   -30.12271337
.float		   71.41650702 ,  -113.84252111
.float		 -168.65907883 ,   135.17821570
.float		   55.15111158 ,    33.41156479
.float		  -15.68273024 ,    -2.26254414
.float		  -16.43235813 ,    33.16395948
.float		  -32.37561524 ,  -126.68427028
.float		  -64.41131117 ,   -39.46784424
.float		   63.17120085 ,  -142.02121282
.float		  -63.89631930 ,   -96.75631058
.float		  -43.23542958 ,    -3.05487814
.float		   62.80051673 ,     5.43570924
.float		  -20.66083226 ,   -53.19518417
.float		 -103.26509638 ,    25.07021414
.float		   77.00739831 ,    54.82338454
.float		  -10.45450838 ,   124.20855326
.float		  -57.49570130 ,    16.69950705
.float		   18.22181163 ,    -3.48020105
.float		   48.69343745 ,   -30.77770396
.float		   55.91766783 ,   120.82640519
.float		   22.77602804 ,    -6.74582018
.float		  -53.35496161 ,   -16.99007357
.float		   48.56080365 ,   -77.71944907
.float		  -35.36208950 ,   -81.73542317
.float		  182.76262292 ,   104.74719560
.float		  -33.52800147 ,     4.52951142
.float		   54.76486673 ,    60.06471212
.float		 -150.71518126 ,   -48.21544986
.float		   68.24107400 ,    42.70233848
.float		   22.72824804 ,    22.20549995
.float		  -79.97595337 ,  -165.01681604
.float		  -10.32437648 ,  -125.44125008
.float		 -107.93784072 ,   -47.61557050
.float		  -86.67281470 ,    49.34738493
.float		 -106.79353455 ,  -236.29629005
.float		   14.81052118 ,   114.22138393
.float		  -73.63480011 ,   -21.56356198
.float		  120.94993297 ,   -48.76875483
.float		 -117.36420773 ,   -49.77843027
.float		   75.15064084 ,   -39.96943866
.float		   48.81487904 ,    72.59387897
.float		  -26.16307389 ,   141.73705712
.float		  161.39229165 ,    78.58315201
.float		    7.04785550 ,   -48.39570829
.float		   49.80896925 ,   -21.13807722
.float		  -88.84867273 ,   203.37022819
.float		   63.88705577 ,    -1.19440170
.float		    8.79982866 ,     7.16416232
.float		   16.10119334 ,    38.37311999
.float		  -80.71899324 ,    69.87988796
.float		  -60.76017891 ,   -47.72037961
.float		  -34.66709319 ,   -78.69150727
.float		   75.69974848 ,    11.72330616
.float		  -41.99642300 ,   -18.60432519
.float		   95.11309771 ,   -11.21372778
.float		   22.13185214 ,   -56.78159756
.float		 -115.41825160 ,    20.00501485
.float		  -40.39102543 ,   -12.09495923
.float		   88.88150077 ,   102.12109868
.float		   27.91573977 ,   -38.67143413	##group 2
.float		   21.60574698 ,    11.77829088
.float		    9.92718410 ,    15.30092277
.float		 -134.21530476 ,  -178.64021277
.float		 -142.63929701 ,     6.72033457
.float		   37.33286486 ,    65.28379666
.float		   72.84547629 ,   108.40393775
.float		   -5.77286729 ,     7.36152844
.float		   39.19436860 ,  -119.70319445
.float		  -34.19927335 ,  -110.00404399
.float		   87.34127914 ,   -19.13940519
.float		  -62.92040837 ,   111.63381524
.float		   69.86328956 ,   -11.25610852
.float		 -100.99600848 ,    44.83173810
.float		  -50.78772619 ,   154.00762201
.float		   85.17779883 ,   -18.01592387
.float		   51.71481428 ,  -103.28205122
.float		   49.11587469 ,   -55.67527214
.float		   76.63606657 ,    86.99972565
.float		  116.32835513 ,   100.56767085
.float		  154.17543701 ,   -19.95518220
.float		  -25.40456618 ,  -116.10164500
.float		   -4.32405987 ,   -60.26738664
.float		  116.06705298 ,     5.51228449
.float		    6.87731292 ,   -26.15046195
.float		   61.80446482 ,  -123.47727320
.float		   52.85019290 ,   151.93254383
.float		    3.76668748 ,    80.84487010
.float		   66.78266070 ,    57.27766764
.float		   10.66914405 ,    67.31765760
.float		   10.77459683 ,    29.96550468
.float		  -71.54059689 ,    34.01392253
.float		   14.50378292 ,   -90.50686754
.float		   -6.24253551 ,   -43.25751885
.float		  127.55656463 ,   -44.32096270
.float		    7.04081320 ,    66.81047476
.float		    3.10565373 ,    81.18925708
.float		   57.92114305 ,   -54.47871355
.float		   31.64571432 ,   -90.05443776
.float		   17.84612805 ,    -9.34422488
.float		  -49.40349024 ,   159.24170918
.float		   31.81508082 ,     8.07011469
.float		    5.97149033 ,    37.73582559
.float		 -116.05323771 ,    52.98419042
.float		  108.30718663 ,    21.78660850
.float		   67.74879205 ,    63.43900758
.float		   -4.39383666 ,   -26.96832081
.float		   75.26123162 ,  -141.91298598
.float		  -44.36551905 ,    24.65146924
.float		   87.91242297 ,   -81.33083092
.float		   17.19303541 ,   -21.76830382
.float		   24.27342503 ,    -1.55590143
.float		  -27.42261747 ,   -29.73370751
.float		  -23.91720899 ,   138.33113656
.float		   36.23349451 ,    19.74609981
.float		   -9.35037058 ,    53.27967221
.float		   45.19915277 ,  -103.08094727
.float		   21.03245122 ,    34.16266362
.float		   61.18896609 ,   179.22284220
.float		   52.46596374 ,  -114.21939241
.float		  -21.27421962 ,    -4.42364323
.float		   62.88184141 ,    30.31885009
.float		  -85.59552026 ,   169.50749006
.float		  -53.26308899 ,    14.32237812
.float		   29.22438331 ,    25.98327113	##group 3
.float		   58.97988093 ,   -84.28745105
.float		  -98.99437438 ,   -24.49734051
.float		   17.47641093 ,   -52.12246021
.float		  -20.21625172 ,   -12.12685339
.float		  108.94832309 ,   -18.33125578
.float		  -10.06082491 ,  -138.60577707
.float		    2.07940194 ,    10.35288810
.float		   57.37075742 ,    74.55524274
.float		  126.22551374 ,     5.30468303
.float		 -147.92334145 ,    74.50350867
.float		   -7.60853455 ,    33.18969002
.float		   97.34000825 ,  -240.08173104
.float		  156.94740782 ,     6.26454390
.float		   -7.91051838 ,    18.33924613
.float		   12.76771959 ,     6.77005346
.float		   -6.67196486 ,   108.56659772
.float		  -11.88208753 ,    61.45888963
.float		  -68.21459274 ,  -149.37736660
.float		    1.98745485 ,    14.62841759
.float		   38.86521713 ,   158.18132260
.float		  -44.47165632 ,    96.68500251
.float		   71.86179716 ,    80.84805051
.float		  -14.61502491 ,    64.98275237
.float		  116.70250538 ,  -107.16464152
.float		   46.07614709 ,    88.75761780
.float		   -3.62041939 ,   -56.51051408
.float		  -53.22253495 ,    55.40057848
.float		  -31.28991703 ,    91.18452222
.float		 -112.40475126 ,  -205.83891934
.float		    9.40247765 ,     7.96050883
.float		  -54.04642147 ,   -33.93438213
.float		   -1.20481014 ,    45.09940222
.float		  -20.38972123 ,   -11.89576503
.float		  -46.03043239 ,    50.01669824
.float		    6.86887553 ,   -47.95305435
.float		  -40.22413868 ,  -146.17497145
.float		   -5.51070132 ,   -55.71613650
.float		  120.78213314 ,    94.40132083
.float		 -111.31356265 ,    60.51580102
.float		   81.66280060 ,   -49.17613221
.float		  -80.50180803 ,    25.16595366
.float		   63.41620759 ,    74.11527007
.float		 -142.05633932 ,   -35.95113490
.float		  -51.01917924 ,    20.29150968
.float		   -4.48309403 ,   -43.20710661
.float		  217.69419578 ,   -82.23167891
.float		  -15.58653517 ,    88.02386582
.float		   -8.46834151 ,    45.97047434
.float		    4.93668287 ,    -8.60155678
.float		  -38.51791227 ,   -20.70234958
.float		   63.67800258 ,   106.98898774
.float		 -180.18796476 ,    24.01755093
.float		   74.45012400 ,   -57.21786864
.float		  167.42526876 ,   -16.93193560
.float		   60.78081904 ,   -80.50978183
.float		   -8.43170158 ,   -41.42634145
.float		   15.98546186 ,   -52.36559529
.float		   95.25678262 ,    79.63987564
.float		   12.80651703 ,  -101.65868748
.float		   44.25024341 ,   -85.53888965
.float		  -89.17145720 ,   108.00965066
.float		    5.56076799 ,   -74.56483351
.float		  -27.44323092 ,   -26.23259168
.float		 -100.95824920 ,   -35.43133471	##group 4
.float		  -63.29403025 ,  -114.11020783
.float		    2.18070462 ,    45.20902100
.float		   42.34281527 ,   -98.28012324
.float		   41.68903340 ,    13.32839345
.float		   -6.77285485 ,    42.85345020
.float		  -42.92818239 ,   101.37142417
.float		  -91.59807744 ,    36.12878489
.float		   30.44062990 ,  -139.54578795
.float		   -8.07894120 ,   -10.56056970
.float		  117.47325926 ,   172.35491168
.float		    5.15594254 ,   -72.74304794
.float		   43.46012091 ,  -170.55046016
.float		  -78.69106142 ,   -30.95420567
.float		  -47.28743806 ,   -25.07970435
.float		  -62.10773963 ,    53.81211779
.float		  162.26696588 ,   -30.37629274
.float		   -0.08109865 ,   -39.42757019
.float		  -98.50625297 ,    28.77668952
.float		   -9.67210687 ,    69.39879735
.float		 -113.96546602 ,   -47.09163700
.float		 -128.16255132 ,    27.17056336
.float		   88.99300818 ,   -23.01537865
.float		  -83.03473335 ,   -27.35269486
.float		  -88.56162616 ,  -143.78796220
.float		 -169.37478522 ,   -15.28247230
.float		  -58.86439647 ,    85.95153009
.float		   29.12444759 ,     3.63531803
.float		  -41.29878304 ,   127.15230836
.float		  -73.42839803 ,   129.95221587
.float		  -24.26457211 ,   -24.65524891
.float		   12.32370958 ,    28.39150147
.float		  -22.41872719 ,  -119.93036387
.float		  -45.32865789 ,    -2.13468370
.float		   62.85662111 ,    -8.60739797
.float		  152.15121780 ,     1.75161498
.float		  110.00165750 ,   -32.51432054
.float		   24.19990799 ,   -38.86899931
.float		   82.48892625 ,   -11.87028748
.float		  112.16960253 ,    23.69382515
.float		  -22.46316234 ,   139.09507127
.float		  104.46271452 ,   -52.01827210
.float		   17.23139546 ,    62.51325889
.float		  -69.76709684 ,    40.71649809
.float		   28.59191430 ,  -107.20393687
.float		 -135.09300424 ,   -14.18961223
.float		  -64.38711607 ,   -55.21494527
.float		  -38.00206655 ,    72.95024630
.float		   80.30506140 ,   -34.71659296
.float		   50.35155744 ,  -104.47524502
.float		  -90.47792248 ,   -89.03467637
.float		  -52.66657748 ,   -46.62416630
.float		   48.36148938 ,    16.05204635
.float		  -60.70357715 ,   229.64772654
.float		   49.99700774 ,   -74.38250007
.float		  -99.50435934 ,    51.40592129
.float		  121.41244321 ,   161.79021047
.float		  -35.40231642 ,   -65.26377876
.float		  -39.75113054 ,   -86.12222811
.float		  -88.40999450 ,  -148.54757033
.float		 -133.12276181 ,    63.74659646
.float		   34.55455320 ,   -78.23540322
.float		  -82.78054830 ,    25.55207048
.float		  114.70362811 ,     1.78452369
.float		  -27.62576361 ,    -9.03379570	##group 5
.float		  -96.05322599 ,    18.24516973
.float		   64.25158932 ,   -62.22691124
.float		  -43.59256655 ,     8.57767698
.float		  -76.87036930 ,    80.99313105
.float		   38.33408312 ,    16.94326326
.float		  -77.57571353 ,     4.92875509
.float		  -48.23268093 ,   -89.65917084
.float		  -66.94354606 ,    14.60128623
.float		   76.60272871 ,  -103.31453579
.float		   57.96380126 ,   131.28412349
.float		  -23.23589061 ,   -75.03425929
.float		   77.69688974 ,    82.24481225
.float		  -35.12864552 ,    11.03185615
.float		 -107.17355662 ,    22.05857281
.float		  -78.84263759 ,    30.90039675
.float		 -101.48237079 ,     1.33906434
.float		  -34.39252735 ,   -27.12062422
.float		 -115.26298708 ,  -138.28596886
.float		   97.64578199 ,   -69.78065157
.float		  -22.82966294 ,    38.46765894
.float		   34.26208580 ,    52.84465642
.float		   45.63841174 ,   113.20004058
.float		   37.51209306 ,    69.63536185
.float		  -55.25401873 ,   -17.71085367
.float		   52.10546386 ,    74.94657156
.float		   15.71830347 ,   -38.77050954
.float		  217.85619523 ,    76.11424670
.float		   75.69054633 ,   -84.40925806
.float		   28.81977811 ,   -39.01818066
.float		   27.93570781 ,    52.68417785
.float		  -81.76716368 ,   -25.43163355
.float		   52.63868905 ,    71.60703230
.float		   29.39164521 ,    23.90080295
.float		  155.93850108 ,    39.20844296
.float		   14.01734040 ,   104.07804637
.float		    1.53579463 ,   -59.73610355
.float		   86.56748520 ,  -113.67178943
.float		  -76.52174567 ,     1.33550216
.float		  162.65331433 ,    40.26931074
.float		  121.78733580 ,    49.71107206
.float		   44.91325591 ,    45.69755420
.float		   98.46394530 ,   -64.57914843
.float		   30.32563269 ,   -17.36105802
.float		    0.66953776 ,   -19.76253638
.float		  -85.24209199 ,   173.88418134
.float		  143.67635038 ,    98.03588240
.float		 -136.70655062 ,    -5.41409378
.float		  140.02407339 ,   -20.75981568
.float		  131.19722248 ,   101.03368724
.float		  -21.21881499 ,   -14.86670800
.float		   18.16647589 ,  -131.78138820
.float		  -78.49413512 ,   -12.20335970
.float		   18.52493920 ,    89.15748332
.float		  -53.07683264 ,    29.29550895
.float		  -35.60058605 ,    44.56433103
.float		   -3.15910417 ,    57.91114177
.float		  128.73189166 ,    10.19459735
.float		  213.60386857 ,    -7.97337777
.float		  129.48398408 ,   -67.11815479
.float		  -82.12790290 ,    42.22980370
.float		   46.24307583 ,   -97.74637414
.float		   47.68753825 ,    77.89712329
.float		  -42.43613794 ,    88.07169817
.float		  183.04975717 ,    72.13742575	##group 6
.float		  -57.83935896 ,   -52.74895733
.float		   33.49920118 ,    67.72035016
.float		   12.12103174 ,    51.22398367
.float		 -138.96175966 ,    19.84636896
.float		   49.04672816 ,    21.44168480
.float		   46.62077942 ,    46.98021462
.float		 -128.78763170 ,   -66.81794270
.float		  -42.87165256 ,    10.73394057
.float		   67.43844626 ,  -120.17610694
.float		 -149.54836750 ,    -3.02473001
.float		  -11.41049105 ,   -25.17684191
.float		   80.94854061 ,   -64.92797585
.float		  116.63879725 ,   -98.91436302
.float		  -91.24071041 ,    37.22571808
.float		   97.08972409 ,   -73.16731700
.float		  -82.65111876 ,   -46.41392975
.float		   37.08155173 ,    24.24745905
.float		  -25.53965012 ,   170.64657734
.float		  122.78733492 ,   104.37499023
.float		  -86.75968907 ,    33.51545348
.float		  -53.75301095 ,   -30.90123577
.float		  -22.38177423 ,   -82.68830884
.float		  109.50580523 ,   -39.27375277
.float		   11.67864798 ,   193.45232997
.float		   22.60432917 ,   143.81769057
.float		  128.24120107 ,    29.39317805
.float		  -85.84237977 ,   -38.72867244
.float		   50.65676052 ,    86.28501016
.float		  -86.59940276 ,  -107.90768640
.float		  218.67872599 ,    55.83304365
.float		  -99.40270217 ,  -204.57307996
.float		  -50.81587468 ,  -113.58807745
.float		  -26.64476725 ,    85.73942948
.float		 -111.96898593 ,     8.58654199
.float		   -0.48831809 ,   -42.73408352
.float		   39.26331492 ,    34.44966433
.float		   -5.53014818 ,    49.83831010
.float		   50.18668322 ,    92.06311807
.float		   64.36827118 ,   -13.32939057
.float		   48.88502209 ,    39.18706663
.float		  -34.85124379 ,   -26.75120678
.float		   73.39546634 ,    20.89029323
.float		  -83.60391147 ,   112.48530714
.float		   48.62259629 ,     8.11206634
.float		   34.99418090 ,   -19.74839741
.float		  -79.43410820 ,    10.21325581
.float		   73.45295583 ,   -92.87545158
.float		   25.80568083 ,    95.28914432
.float		   36.80269043 ,   -65.82856443
.float		   60.68693754 ,    51.65987041
.float		 -109.40247599 ,     2.84239981
.float		 -145.72733480 ,    26.12740258
.float		  -39.74791742 ,   103.40798151
.float		   40.16711420 ,  -108.37424327
.float		    7.05820100 ,    29.90205507
.float		    7.00567387 ,    82.64488498
.float		  -15.25923751 ,    49.29491696
.float		  -88.42819156 ,  -106.39871547
.float		  -83.81306137 ,   -25.37310815
.float		  -38.48105954 ,    24.16530527
.float		  -94.91780346 ,     6.58619885
.float		  -48.25874386 ,  -113.38288039
.float		  -59.30904995 ,   -48.95739644
.float		 -109.18378752 ,   -25.82473721	##group 7
.float		 -136.31178820 ,    11.68472911
.float		  -38.93893777 ,    -0.64836436
.float		 -116.97405846 ,    60.62055796
.float		  -35.84542761 ,     7.52642458
.float		   -1.55106369 ,  -115.59776128
.float		 -156.30479052 ,    39.06034113
.float		  -62.81653009 ,   -72.65863851
.float		 -105.53393604 ,    58.98553378
.float		   46.28136245 ,   -66.22738894
.float		   32.23292819 ,   -39.14009716
.float		  -31.96697411 ,    -6.52460551
.float		   22.28568487 ,   -78.18037173
.float		 -103.95126650 ,    89.88552842
.float		  105.56832824 ,   -61.71261639
.float		  157.02474790 ,    -3.80181880
.float		 -136.77208022 ,    14.65243412
.float		   -8.02890449 ,   -57.38370295
.float		   22.74044426 ,    84.69889957
.float		 -100.20109389 ,    61.57982056
.float		  -67.79903535 ,   -27.18079157
.float		  -59.82663232 ,  -144.93407787
.float		 -174.46990969 ,    54.91777280
.float		 -129.49133127 ,    92.99865262
.float		    6.71617280 ,   -46.35096649
.float		   35.73270746 ,    56.25329530
.float		   71.93965674 ,   -40.47551730
.float		   67.04649116 ,   103.75620903
.float		   18.97836483 ,   -62.26510327
.float		 -119.10093721 ,    58.45638212
.float		  170.87122019 ,    54.32408952
.float		  129.14569279 ,    21.16973448
.float		  108.06736087 ,   -51.35695972
.float		   54.00475417 ,  -122.69395837
.float		  -33.66235059 ,    -2.76199486
.float		  123.84223627 ,   -45.36524913
.float		  -19.62106986 ,    70.09110626
.float		   74.23675220 ,    82.03012927
.float		    9.16245684 ,   -11.24311816
.float		 -119.92344115 ,   -85.64662678
.float		  -89.97862232 ,   211.36282973
.float		  126.50391212 ,   156.57919865
.float		  -27.62736484 ,   -98.77292852
.float		  -44.08953243 ,  -142.46365510
.float		    0.00264525 ,   -20.63052742
.float		 -134.03740775 ,    -3.11024424
.float		    6.67473612 ,   112.62334148
.float		 -131.37679653 ,    -1.89551117
.float		  -40.14470740 ,  -126.60631681
.float		  182.38454950 ,    35.53848462
.float		  -53.89596187 ,     1.41661303
.float		  179.44547568 ,   112.29464997
.float		   11.67061228 ,   191.83005810
.float		    7.64925331 ,   113.56221821
.float		   54.84500371 ,   -40.65701744
.float		   68.43702091 ,   168.95961441
.float		  -46.67286938 ,   -49.50323872
.float		  -15.85847433 ,   -53.20139169
.float		   71.48381633 ,    51.88169351
.float		   64.07582229 ,   133.50476429
.float		   47.04839334 ,    64.58326373
.float		 -210.66477877 ,    -5.91212797
.float		  132.96659634 ,   -41.09658929
.float		  -55.95391095 ,  -122.93560716
.float		  -81.97233616 ,   124.09329344	##group 8
.float		  -71.24203265 ,   -22.87507993
.float		   33.13061170 ,  -111.25549501
.float		  -27.40143701 ,    74.04090384
.float		  -28.33179370 ,    -1.67346347
.float		   69.45660266 ,   268.15445391
.float		   86.34819308 ,   -78.64591314
.float		  -67.33654830 ,   -90.41183241
.float		  -11.87024682 ,   -45.73702424
.float		   73.09821115 ,    19.47882275
.float		  -30.79948728 ,   -53.75049393
.float		  -59.71751673 ,   -43.53656938
.float		  -42.84757487 ,  -155.15613193
.float		 -108.39111140 ,   -97.34092426
.float		   29.86955030 ,   138.69830273
.float		  -52.23818249 ,   -81.41224612
.float		   23.66060161 ,   -20.71514778
.float		  -24.48832727 ,   -10.82231219
.float		  127.95869870 ,   133.94365159
.float		   11.18136077 ,    33.33066482
.float		  -21.20432442 ,    -7.54743565
.float		  170.47607039 ,    31.13308134
.float		    1.24346245 ,    21.00009007
.float		  -20.37655223 ,    16.70817342
.float		   20.06683840 ,    29.18555054
.float		  -27.82623512 ,   -72.89417815
.float		   79.42065350 ,     8.83276579
.float		  116.46977771 ,   -57.98604294
.float		  109.35328188 ,  -194.74606868
.float		   65.99603414 ,  -152.52552111
.float		   -7.32626444 ,    40.67567411
.float		   37.67554001 ,   -69.23806756
.float		  -29.90114612 ,    79.02671948
.float		  145.72191476 ,     7.87246991
.float		  -52.76605563 ,   -72.94354099
.float		   88.55197972 ,    68.77170696
.float		   33.11350312 ,    17.23624111
.float		   -4.38549969 ,    62.66799258
.float		  -39.66493484 ,    -0.66437701
.float		   45.93115400 ,   -51.31525779
.float		  197.20475067 ,   155.40234468
.float		   56.46903872 ,   -43.08947632
.float		  -31.11983265 ,   -50.04603213
.float		   18.01006804 ,   101.28264179
.float		  -46.18538621 ,  -103.32592609
.float		   63.04058841 ,  -148.35084755
.float		   46.61879350 ,   -16.41749134
.float		    6.66934657 ,  -103.19593372
.float		   -1.30217297 ,    28.63208666
.float		   62.06817214 ,   115.76819584
.float		    5.95071237 ,   -98.21723815
.float		  -78.48388809 ,  -205.67963445
.float		  -60.41359374 ,    23.42112925
.float		  -92.10770821 ,    77.26495517
.float		   32.71444827 ,   -89.73299794
.float		  -20.99427851 ,  -166.44823198
.float		  -39.34233506 ,    40.88263143
.float		 -113.42377090 ,    -2.84334633
.float		  -40.88354149 ,    38.55208527
.float		  220.85595930 ,   -53.39529770
.float		  -25.81430876 ,   -75.62382653
.float		   34.82522350 ,   -59.06894921
.float		   90.10161377 ,    17.45392341
.float		  -37.91822225 ,    -7.15137584
.float		   -9.55763465 ,     6.57797114	##group 9
.float		   73.61699279 ,   -25.19245625
.float		   37.04462693 ,  -223.26777150
.float		   52.09683371 ,  -112.50519825
.float		   77.09324475 ,    57.01535790
.float		  -53.42732356 ,   -20.30131742
.float		  -64.74520276 ,   126.35850657
.float		   98.89498009 ,   -17.08327651
.float		  -17.40990140 ,   -26.76080244
.float		   -6.62672686 ,    -0.67372416
.float		   56.77473783 ,     4.80758260
.float		    3.56447922 ,    -4.97986959
.float		   57.65637017 ,   105.71095312
.float		  -28.58112912 ,    86.63715710
.float		 -107.80013439 ,    22.08083915
.float		 -130.38095759 ,   -50.64521397
.float		   55.90962139 ,   -15.30042712
.float		   91.32120720 ,    -8.25783606
.float		   63.03861646 ,    63.27887186
.float		   58.13898646 ,   126.40750250
.float		   66.14794439 ,   -24.01048069
.float		  -77.56992643 ,    13.14755838
.float		 -142.67983060 ,    -5.46994078
.float		  123.44134400 ,   -16.56256914
.float		 -133.34539582 ,    89.58655042
.float		   68.21003344 ,   -98.94527208
.float		  -95.06444671 ,   -75.19265046
.float		   63.75204252 ,   -75.07125647
.float		   29.67704870 ,    -2.38727084
.float		   28.38645063 ,    10.50992639
.float		  -57.29566397 ,  -223.33120684
.float		    5.11067484 ,   -54.90815430
.float		   32.43299177 ,    54.00557176
.float		  -16.62458829 ,    96.17657829
.float		   27.43832212 ,  -122.37722021
.float		  154.90091673 ,  -109.30396278
.float		   -8.36635961 ,   110.34137968
.float		   99.05573833 ,     9.25111072
.float		   76.84988993 ,   -50.61084857
.float		   47.60200909 ,    25.98515745
.float		   39.42094004 ,    17.59962360
.float		   69.94521339 ,    28.28431155
.float		  187.96722388 ,   -19.73042210
.float		   47.98012842 ,    34.40927430
.float		  -24.74158889 ,    48.08553689
.float		   58.46002357 ,   123.84219036
.float		   31.39337222 ,   -19.39431035
.float		  -10.00071497 ,    63.25909604
.float		   90.10791833 ,    41.51450819
.float		 -142.38659210 ,   -11.84445398
.float		   12.14250142 ,   112.53520105
.float		  -95.82919647 ,  -164.63265304
.float		   74.09975796 ,   -40.12666068
.float		 -129.48904457 ,    98.63174786
.float		  -99.09601819 ,   -32.38580695
.float		   46.54262646 ,   105.03584790
.float		  -31.30467817 ,   -14.87766882
.float		  -71.36112863 ,    -9.80416645
.float		  -88.68388134 ,   110.16570358
.float		  -31.52429567 ,   -92.39792326
.float		   51.48325226 ,    71.03098898
.float		  129.87152186 ,   103.69842817
.float		   89.27011607 ,   -31.70434726
.float		   35.59663652 ,   108.02157281
.float		  -25.01241146 ,   -12.76882359	##group 10
.float		 -147.79853122 ,     8.65369186
.float		   99.01264194 ,   -63.76534419
.float		  125.67901150 ,   -64.68464324
.float		 -171.03598601 ,   -60.40095488
.float		  -73.16069550 ,    50.86879482
.float		  148.54049747 ,   184.01704277
.float		   81.50923960 ,    52.02959021
.float		  -41.78305457 ,   -78.52880446
.float		   77.87253024 ,    -7.89933432
.float		  -35.23277157 ,  -116.36462217
.float		  -48.83891137 ,     3.44430895
.float		 -113.75180420 ,   132.64699236
.float		   47.46634115 ,     0.82468872
.float		   54.62437387 ,   -53.48368648
.float		  -73.63080320 ,  -150.70225780
.float		   71.04530913 ,    49.86161181
.float		  -17.19331548 ,    60.44755273
.float		  -36.83825090 ,   103.62432282
.float		  -83.71347858 ,    73.97665724
.float		  108.72265998 ,   -40.55340502
.float		  -50.84365954 ,   139.80329303
.float		  124.99197382 ,    74.57793777
.float		  -92.63881554 ,   -82.98264657
.float		  104.82689343 ,   -35.61982571
.float		  -12.67732266 ,    72.99118508
.float		   36.37681121 ,   -15.97945503
.float		  -16.00273742 ,    27.25434860
.float		   69.12215747 ,    80.48230214
.float		  -86.77011672 ,   -20.09698641
.float		  -53.90558003 ,  -107.70759154
.float		   62.81725015 ,   -80.32820571
.float		   23.00209521 ,  -186.92877153
.float		  210.65801824 ,   107.35001832
.float		  -80.58093316 ,     3.46113457
.float		  -81.66047477 ,     9.78691714
.float		  -84.55053102 ,    27.36901827
.float		    6.27558803 ,   -41.66471704
.float		  198.12709100 ,    -1.02237055
.float		 -122.46774386 ,    77.25928906
.float		  143.08604252 ,     0.48263858
.float		  -38.09439900 ,   -41.81790755
.float		  -35.59979779 ,    46.86237718
.float		   71.20425768 ,    -9.85109362
.float		  -14.20331238 ,     4.26924870
.float		   15.78150198 ,    55.65521163
.float		 -149.33885541 ,   -37.81479454
.float		  -90.03841934 ,   -72.44233661
.float		   17.20356291 ,   130.48931373
.float		  -54.55608250 ,   -40.85030447
.float		  -18.80469314 ,    94.16927910
.float		  146.44807164 ,     8.53504766
.float		    3.88102189 ,  -165.08468374
.float		  -31.68204270 ,   -62.99497748
.float		   33.72435288 ,    28.34320973
.float		  -23.48057540 ,   -60.74054247
.float		  122.92632455 ,   106.46738362
.float		  106.89221097 ,   -60.60404545
.float		  118.68123130 ,    11.70768504
.float		   27.10236929 ,   -67.25109104
.float		  -77.34396676 ,   -28.75198013
.float		   53.66900461 ,     0.77536048
.float		 -114.82941718 ,   107.25778786
.float		  123.22949839 ,   -81.96170734
.float		  -34.40445018 ,   -74.61402961	##group 11
.float		  -62.38914685 ,   -60.43686942
.float		 -107.96439541 ,    71.28720970
.float		  -65.23693769 ,    -4.32627625
.float		   76.94107882 ,    12.87757357
.float		  -59.39661344 ,   122.39914351
.float		  -52.22893265 ,   -56.95723130
.float		   27.19991964 ,    30.79253378
.float		 -100.17808279 ,   -53.66989632
.float		  102.09551637 ,  -127.45677926
.float		   33.56745116 ,  -102.12555258
.float		  -59.74393817 ,    73.34526310
.float		  -26.57190567 ,   182.11521442
.float		 -130.55501501 ,    90.73168215
.float		  -33.35358579 ,  -103.63727875
.float		  -20.55105550 ,   -25.12423289
.float		    8.60805579 ,    11.64487368
.float		  -65.25966668 ,    -7.07838869
.float		  -97.02866594 ,   -23.38196808
.float		    2.04139983 ,   -13.02975848
.float		  -89.99811953 ,    57.53515653
.float		  -23.32556951 ,   -35.10090400
.float		  -98.83693476 ,   153.46606494
.float		  -64.18376719 ,  -109.26103252
.float		   34.43973257 ,   -56.14099630
.float		    3.96642030 ,    77.86516527
.float		   65.88360011 ,   -79.99649627
.float		  -91.52771479 ,   -36.64501614
.float		   77.56543071 ,   143.21870430
.float		  -77.71383427 ,  -117.58462277
.float		  -29.13152846 ,   103.89478777
.float		  -21.05169538 ,   -21.04114216
.float		   19.54146367 ,    12.82865996
.float		  -24.05219741 ,    59.67317963
.float		   -9.85855413 ,     5.37951807
.float		   72.50683892 ,    -2.77037779
.float		   68.29701256 ,   -28.01601049
.float		  -19.36428931 ,     6.94124025
.float		   37.07635091 ,    42.60262200
.float		  -80.38325413 ,    69.14436216
.float		   75.42006418 ,   107.25347687
.float		  -50.71588831 ,    20.78647406
.float		   71.32538775 ,    -0.41107634
.float		  111.93975773 ,   -50.54578669
.float		  -34.01713162 ,  -111.53490515
.float		   22.46946331 ,    -6.49447641
.float		  -16.03760444 ,    98.36984060
.float		   18.74687336 ,    56.50244975
.float		   52.01875962 ,   145.13273666
.float		  -69.95864338 ,    11.95987641
.float		   40.16508797 ,    39.02454713
.float		  -22.28399625 ,  -154.13157785
.float		   79.06207104 ,  -116.10546536
.float		  -42.68195021 ,   103.12918382
.float		  -36.44198601 ,    74.76879741
.float		   89.86867506 ,   104.52556491
.float		   69.26230931 ,    88.50670912
.float		   86.37464311 ,    13.51223811
.float		    0.68425925 ,  -140.32335618
.float		   17.82108271 ,    -7.98842538
.float		   70.83416481 ,   -15.38068779
.float		 -108.03170732 ,   -33.19205780
.float		  136.67343210 ,  -148.78298515
.float		  -81.13983981 ,   151.57198346
.float		  159.08293637 ,    27.51307675	##group 12
.float		  108.73827636 ,    78.20559690
.float		   -7.65105221 ,   -35.09999196
.float		   23.30483261 ,   220.29974662
.float		    7.97449202 ,    10.53829693
.float		   40.30202343 ,   -10.22503499
.float		  -34.67511219 ,   109.66533460
.float		    5.04458102 ,   -92.75678831
.float		  -10.15454074 ,   -83.14228223
.float		 -184.77236346 ,     4.09997933
.float		  -44.03080074 ,   -26.08408253
.float		 -124.25198528 ,   -88.07683691
.float		 -112.12274210 ,    89.99796415
.float		  140.98294763 ,    81.41979480
.float		  -97.02402414 ,   -22.95822062
.float		 -189.66669338 ,   -37.33474951
.float		   88.69657746 ,  -114.52114814
.float		 -134.22472531 ,    19.27674564
.float		 -137.81946066 ,   -51.65248262
.float		 -175.37377329 ,    55.21987789
.float		   22.78808592 ,   -66.49117295
.float		   91.65059397 ,  -114.67791385
.float		  -76.02520724 ,    -3.36277947
.float		   50.66926168 ,   -40.62927225
.float		  -12.49082719 ,  -116.79424728
.float		 -105.86095993 ,    53.29138380
.float		 -168.95194909 ,  -259.02868887
.float		   58.65207532 ,   -73.58590552
.float		  118.01973374 ,   -33.35055446
.float		   27.97426522 ,    20.10180997
.float		  -51.79027274 ,    13.38847445
.float		   62.93837111 ,    66.62010193
.float		   57.45327614 ,    -8.71322842
.float		  -80.86541803 ,    39.01276354
.float		 -132.66437877 ,    13.14483120
.float		  -67.95742903 ,    61.32913155
.float		   47.42358511 ,    58.61939416
.float		 -116.01813028 ,   108.43404161
.float		 -111.04328826 ,  -128.21305184
.float		   23.18114995 ,   -76.19774489
.float		  -81.81332566 ,   -60.77724062
.float		  141.03635146 ,  -123.02922945
.float		  -38.31819201 ,    -0.33991824
.float		   60.83901265 ,  -167.41511003
.float		  -18.45768238 ,    50.88569990
.float		   96.75873387 ,    46.81365276
.float		   56.33051053 ,    87.92521853
.float		    4.32900780 ,    87.07604933
.float		  151.07526465 ,   -16.35886459
.float		  -58.94169033 ,    32.37674416
.float		   77.09147101 ,    16.57650919
.float		   31.14289618 ,   -92.56831214
.float		  -49.92957755 ,   -14.61909130
.float		   74.80544170 ,   -45.34681095
.float		   61.02186360 ,    76.61335674
.float		   28.68445563 ,  -102.37043370
.float		   36.94218849 ,     0.52454212
.float		  -14.65213105 ,     3.91702321
.float		  -44.14239371 ,   -35.00459142
.float		   17.31025844 ,    97.32501351
.float		  -94.73919899 ,    58.61380778
.float		    0.46114014 ,    26.56286753
.float		  110.79160219 ,   -52.28217161
.float		  108.40372630 ,    73.38784960
.float		  -25.48910690 ,   -37.37387861	##group 13
.float		  -20.35144612 ,  -126.83748563
.float		  -84.95279120 ,   -67.78450782
.float		   58.94772296 ,  -180.57039468
.float		   41.31516476 ,   -48.11796531
.float		   21.09112370 ,   -15.74819661
.float		  116.36486435 ,    49.95286738
.float		   -5.43411505 ,    -5.83446825
.float		   90.55320171 ,   127.45242663
.float		  160.94178876 ,  -116.32474703
.float		   -0.31793632 ,   -67.62076776
.float		  130.09267916 ,  -145.79041723
.float		   58.58070291 ,  -117.27774681
.float		   -0.91018860 ,   -81.61972484
.float		   69.49692980 ,     1.31268009
.float		   93.10357497 ,   111.92222641
.float		 -136.34920863 ,  -147.59454061
.float		  -47.22904520 ,    -3.47857619
.float		  108.46546162 ,    25.32245774
.float		   53.36810531 ,  -121.55885593
.float		   73.94828564 ,   -10.86139596
.float		  148.93079360 ,    47.81896804
.float		  -87.60031466 ,   -78.73381081
.float		   13.91932200 ,    62.76519647
.float		   74.62010301 ,    97.40607973
.float		  -18.42718432 ,    68.93993692
.float		    5.49973564 ,    15.20056852
.float		   12.34335704 ,    28.14239236
.float		   58.91482601 ,    89.53840756
.float		   36.88556328 ,   -85.10730363
.float		   10.27895484 ,   -66.33844394
.float		  -73.32607330 ,  -111.76324831
.float		 -142.57130854 ,    13.94716395
.float		  -82.62642701 ,   -21.34478718
.float		 -108.62790098 ,   -23.81006504
.float		   -4.03241511 ,    -8.35340297
.float		   10.32624481 ,   137.27069245
.float		  107.59219798 ,   216.62612373
.float		 -161.78840469 ,   -52.11823470
.float		   64.94598608 ,  -103.47824954
.float		 -113.79998539 ,   113.42012529
.float		 -108.51064290 ,    27.79660470
.float		   29.25342711 ,   -69.94077683
.float		  -39.38038314 ,   126.39327112
.float		   70.17283536 ,    18.66764235
.float		  -84.25556528 ,   -22.00701386
.float		  -56.31090518 ,   -24.72204367
.float		   29.93219967 ,  -113.17712550
.float		   19.06218738 ,    98.83098018
.float		  109.46611936 ,   -32.94568061
.float		   16.22917413 ,   -13.77563787
.float		   -0.29725964 ,    14.80250705
.float		  -11.91452962 ,    29.66616077
.float		   35.37912055 ,   -28.58184396
.float		   91.57225887 ,    80.75434207
.float		 -180.62964510 ,   -71.48794812
.float		    4.49442118 ,   -52.52087712
.float		 -126.65115816 ,    20.54227464
.float		   49.55274340 ,    38.71315174
.float		   23.16038468 ,     2.87839117
.float		  -55.53272186 ,  -101.42471336
.float		  -42.84585929 ,   -32.36530822
.float		   32.65112305 ,   -39.01520767
.float		  -61.09219214 ,   100.33249667
.float		  113.05660730 ,    42.88503994	##group 14
.float		  -99.14743384 ,   -74.83089486
.float		   59.91908998 ,    16.59361119
.float		   -4.90351751 ,    76.76133591
.float		  -82.16693885 ,   -58.30507370
.float		  104.53841063 ,  -149.17124351
.float		  117.45672299 ,   112.95211054
.float		  -56.00089716 ,    31.34165045
.float		 -135.56922261 ,   108.13245684
.float		   73.87948115 ,   -74.17127170
.float		  -67.07427368 ,   110.16159758
.float		  -72.13214387 ,  -190.09138602
.float		   58.59829754 ,    29.30147647
.float		  -73.53269498 ,    10.95453759
.float		  -14.54099866 ,    63.33293695
.float		  -65.82031273 ,     9.39150417
.float		  -28.24512699 ,   -75.67920917
.float		   46.68983036 ,    11.59558921
.float		   95.95932439 ,   -27.50442131
.float		   17.25322228 ,   -65.09141294
.float		  -68.11028158 ,    10.28903817
.float		  138.28722432 ,   -57.09967736
.float		  -84.65276815 ,    80.61749430
.float		   -6.64482678 ,    62.55344318
.float		  -74.97614452 ,   198.30978936
.float		   83.97667838 ,   -46.09053371
.float		   38.07106285 ,   -41.39170766
.float		 -120.49105791 ,   -86.98686557
.float		   85.32983560 ,    54.70877443
.float		   -8.51959507 ,   -94.21325917
.float		  -34.12638407 ,    54.48235477
.float		   98.86901770 ,   -47.99395029
.float		  101.51340197 ,    -7.40091280
.float		  -60.71177876 ,    35.55242460
.float		  186.09532109 ,   -42.30020609
.float		  -19.04028136 ,    62.56671149
.float		  -25.30359478 ,   115.74994484
.float		   42.63728660 ,    52.74457919
.float		  -97.64269534 ,    65.85247545
.float		  -88.65424382 ,   -23.98884878
.float		  -53.55172024 ,    46.85492904
.float		  -60.43821783 ,  -254.10258226
.float		   89.68832568 ,    -1.62317242
.float		 -169.11156038 ,   -57.37269622
.float		 -115.75422487 ,   -27.78022342
.float		  -49.09098858 ,    55.62366052
.float		   32.86459172 ,   -82.27228685
.float		   26.08277950 ,    76.57748650
.float		  -90.04754580 ,   -73.44442979
.float		   12.04543732 ,  -135.46163105
.float		 -157.58294090 ,   -34.30922955
.float		   13.02290229 ,  -178.49394204
.float		  152.73199540 ,     2.94575905
.float		   59.42253601 ,    66.40510613
.float		 -166.41690710 ,  -131.63244855
.float		 -253.42231445 ,    24.72837571
.float		  159.39627598 ,    65.15136160
.float		   24.39081233 ,   154.59451814
.float		 -120.28423906 ,    56.37312782
.float		  123.28443944 ,   140.97375643
.float		  -63.42106588 ,   -22.45052520
.float		  -20.34872085 ,   148.90684605
.float		   64.62162241 ,   177.54582265
.float		   13.32500802 ,    33.47948947
.float		  121.64471104 ,   -94.99393846	##group 15
.float		 -119.86278593 ,  -140.90533119
.float		   26.89059086 ,   158.58874250
.float		   46.17569966 ,   -36.82072462
.float		  -71.97228187 ,   -80.03628085
.float		   44.58800423 ,   -19.06907744
.float		   23.53240934 ,   -92.98235456
.float		  111.55152000 ,   171.35054110
.float		  158.86027661 ,   160.70728715
.float		 -105.57509448 ,   -77.49166655
.float		  -35.39695659 ,   -86.40794837
.float		  -66.03566393 ,   117.83209558
.float		 -187.78906992 ,    80.21591923
.float		  -72.59243319 ,   -68.46052336
.float		  -47.07780937 ,    38.32089746
.float		  -57.13776431 ,  -158.27270090
.float		 -115.61414239 ,    78.62890625
.float		   91.82649139 ,   -73.90184921
.float		  -48.08774548 ,     3.04808291
.float		  -14.99652346 ,   -67.45006476
.float		  -92.92169418 ,  -121.26268570
.float		  -93.81344497 ,    56.07607812
.float		  -39.84533776 ,    49.85026523
.float		   14.28279597 ,    -5.29461561
.float		  -20.36506920 ,   -77.56077487
.float		   26.02220434 ,    69.60245285
.float		  105.12978724 ,   -13.93767538
.float		   96.44387084 ,    19.46549380
.float		  -81.26978384 ,   -53.25028332
.float		   21.19086125 ,   -54.23821626
.float		   -5.42795896 ,    57.00025934
.float		   -9.73114930 ,   -31.32263986
.float		  -95.22298974 ,     9.43649560
.float		   11.51523460 ,   -44.25366972
.float		   95.82712936 ,    14.68807099
.float		  191.23820685 ,   -21.57882354
.float		   59.82123045 ,    -8.66887239
.float		  -59.08254536 ,  -118.93810620
.float		  115.30619853 ,   -15.76354250
.float		 -102.97338615 ,   -27.80374855
.float		   60.34359689 ,   -18.04551287
.float		  -88.23881241 ,    -5.54360071
.float		  -23.95781787 ,    57.88455959
.float		   50.47257595 ,   100.38568932
.float		  -56.41534422 ,   -23.25187301
.float		   13.16086881 ,   -24.84667566
.float		   51.88392493 ,  -155.21949003
.float		 -105.63057351 ,    74.01369061
.float		   14.43297522 ,   -37.62169669
.float		  -13.67713865 ,   -27.84155794
.float		  171.76319897 ,   -32.31437943
.float		   10.97178146 ,    36.49360613
.float		   10.33156784 ,     6.14887963
.float		   96.11651872 ,   208.22754989
.float		  -84.28771422 ,   -85.79871742
.float		 -117.98194912 ,    -7.66614028
.float		  135.63475441 ,   -77.87768134
.float		   32.48579990 ,   -24.57998733
.float		  -30.36571614 ,     1.33214722
.float		  110.00235291 ,   -52.08737920
.float		   -5.24689991 ,    11.34727763
.float		  -61.57964482 ,   -20.83874457
.float		 -173.30034209 ,   -58.58317545
.float		  -79.04759503 ,    94.50419109
.float		  -38.26027554 ,    70.56279740	##group 16
.float		   50.63667382 ,    57.68566466
.float		  -12.94591708 ,    79.14067661
.float		   14.02049724 ,    -3.05445120
.float		  124.62404679 ,    67.19699353
.float		  -64.00031969 ,   -35.32732081
.float		  -14.95082932 ,    77.05045172
.float		   90.24900717 ,   -46.25809860
.float		   77.58584470 ,    50.47402138
.float		  188.79497820 ,    94.45543046
.float		   75.70274377 ,    -3.97440592
.float		  -76.92064269 ,   -62.70104530
.float		   30.12820585 ,  -132.00244606
.float		  -31.57119134 ,    66.62923380
.float		   48.17036933 ,    11.73582782
.float		  106.96991819 ,   -19.05166555
.float		   25.01540985 ,     2.34260767
.float		  -42.54829074 ,    73.42191993
.float		   19.05574653 ,  -130.26216674
.float		  -58.87840206 ,   121.09979392
.float		  -42.89301229 ,    36.49965798
.float		  -14.78133267 ,    73.76886983
.float		  -14.66846809 ,    10.02442652
.float		  -94.83304691 ,   -56.13633769
.float		  -11.90447639 ,    11.72141933
.float		  -46.53684688 ,     3.88729119
.float		 -132.47408951 ,   -33.08809676
.float		    9.23462695 ,  -148.45408723
.float		   68.24890189 ,    -5.88883188
.float		  101.82579898 ,    62.27163866
.float		  -75.28713787 ,    59.83956435
.float		   -5.68173619 ,   113.08918341
.float		  -92.20720295 ,   211.01658501
.float		  111.47696495 ,   -10.99199598
.float		  108.94464539 ,    41.98397899
.float		  -30.99002763 ,  -135.77826688
.float		   26.75960223 ,    69.89585963
.float		  -63.80106607 ,  -198.56515398
.float		   -8.46094639 ,   151.59182344
.float		  118.68372282 ,  -191.98784596
.float		 -118.27855504 ,   114.47899444
.float		  -19.38994453 ,  -104.72485428
.float		   90.07439540 ,  -109.70218078
.float		  -30.19067762 ,   -18.42348328
.float		  -42.31121290 ,   139.35285729
.float		  -14.98563244 ,  -129.48193932
.float		 -221.82565897 ,   -68.30674529
.float		 -138.32846254 ,   -37.50376764
.float		  -71.20050222 ,  -147.33043307
.float		   93.75651585 ,    19.70885062
.float		 -107.07402035 ,   -36.39050171
.float		   36.03387274 ,   -29.50778610
.float		  -82.83528996 ,   192.48747729
.float		  -14.74842017 ,  -111.19604413
.float		  100.94394834 ,   -14.30305050
.float		  -59.56569600 ,    57.08911854
.float		  131.34704653 ,    34.02895841
.float		   27.98077972 ,  -113.12546043
.float		   98.57322069 ,    16.61984296
.float		   70.46829410 ,     5.65147991
.float		   28.55488915 ,   -86.76734223
.float		   94.29523569 ,    25.81170788
.float		   47.04963408 ,    49.93355636
.float		    0.99757850 ,    42.87776185


w_din_dat:
.float		    0.00000000 ,     1.00000000	##stage1
.float		    0.00000000 ,     1.00000000
.float		    0.00000000 ,     1.00000000
.float		    0.00000000 ,     1.00000000
.float		    0.00000000 ,     1.00000000	##stage2
.float		   -1.00000000 ,     0.00000000
.float		    0.00000000 ,     1.00000000
.float		   -1.00000000 ,     0.00000000
.float		    0.00000000 ,     1.00000000	##stage3
.float		   -1.00000000 ,     0.00000000
.float		   -0.70710678 ,     0.70710678
.float		   -0.70710678 ,    -0.70710678
.float		   -0.38268343 ,     0.92387953	##stage4
.float		   -0.92387953 ,    -0.38268343
.float		   -0.92387953 ,     0.38268343
.float		   -0.38268343 ,    -0.92387953
.float		   -0.19509032 ,     0.98078528	##stage5
.float		   -0.98078528 ,    -0.19509032
.float		   -0.83146961 ,     0.55557023
.float		   -0.55557023 ,    -0.83146961
.float		   -0.55557023 ,     0.83146961
.float		   -0.83146961 ,    -0.55557023
.float		   -0.98078528 ,     0.19509032
.float		   -0.19509032 ,    -0.98078528
.float		    0.00000000 ,     1.00000000	##stage6 group1
.float		   -1.00000000 ,     0.00000000
.float		    0.00000000 ,     1.00000000
.float		   -1.00000000 ,     0.00000000
.float		    0.00000000 ,     1.00000000	##stage7  group1
.float		   -1.00000000 ,     0.00000000
.float		   -0.70710678 ,     0.70710678
.float		   -0.70710678 ,    -0.70710678
.float		    0.00000000 ,     1.00000000	##stage8  group1
.float		   -1.00000000 ,     0.00000000
.float		   -0.70710678 ,     0.70710678
.float		   -0.70710678 ,    -0.70710678
.float		   -0.38268343 ,     0.92387953
.float		   -0.92387953 ,    -0.38268343
.float		   -0.92387953 ,     0.38268343
.float		   -0.38268343 ,    -0.92387953
.float		    0.00000000 ,     1.00000000	##stage9  group1
.float		   -1.00000000 ,     0.00000000
.float		   -0.70710678 ,     0.70710678
.float		   -0.70710678 ,    -0.70710678
.float		   -0.38268343 ,     0.92387953
.float		   -0.92387953 ,    -0.38268343
.float		   -0.92387953 ,     0.38268343
.float		   -0.38268343 ,    -0.92387953
.float		   -0.19509032 ,     0.98078528
.float		   -0.98078528 ,    -0.19509032
.float		   -0.83146961 ,     0.55557023
.float		   -0.55557023 ,    -0.83146961
.float		   -0.55557023 ,     0.83146961
.float		   -0.83146961 ,    -0.55557023
.float		   -0.98078528 ,     0.19509032
.float		   -0.19509032 ,    -0.98078528
.float		    0.00000000 ,     1.00000000	##stage10  group1
.float		   -1.00000000 ,     0.00000000
.float		   -0.70710678 ,     0.70710678
.float		   -0.70710678 ,    -0.70710678
.float		   -0.38268343 ,     0.92387953
.float		   -0.92387953 ,    -0.38268343
.float		   -0.92387953 ,     0.38268343
.float		   -0.38268343 ,    -0.92387953
.float		   -0.19509032 ,     0.98078528
.float		   -0.98078528 ,    -0.19509032
.float		   -0.83146961 ,     0.55557023
.float		   -0.55557023 ,    -0.83146961
.float		   -0.55557023 ,     0.83146961
.float		   -0.83146961 ,    -0.55557023
.float		   -0.98078528 ,     0.19509032
.float		   -0.19509032 ,    -0.98078528
.float		   -0.09801714 ,     0.99518473
.float		   -0.99518473 ,    -0.09801714
.float		   -0.77301045 ,     0.63439328
.float		   -0.63439328 ,    -0.77301045
.float		   -0.47139674 ,     0.88192126
.float		   -0.88192126 ,    -0.47139674
.float		   -0.95694034 ,     0.29028468
.float		   -0.29028468 ,    -0.95694034
.float		   -0.29028468 ,     0.95694034
.float		   -0.95694034 ,    -0.29028468
.float		   -0.88192126 ,     0.47139674
.float		   -0.47139674 ,    -0.88192126
.float		   -0.63439328 ,     0.77301045
.float		   -0.77301045 ,    -0.63439328
.float		   -0.99518473 ,     0.09801714
.float		   -0.09801714 ,    -0.99518473
.float		   -0.09801714 ,     0.99518473 ##stage6 group2
.float		   -0.99518473 ,    -0.09801714
.float		   -0.09801714 ,     0.99518473
.float		   -0.99518473 ,    -0.09801714
.float		   -0.04906767 ,     0.99879546	##stage7  group2
.float		   -0.99879546 ,    -0.04906767
.float		   -0.74095113 ,     0.67155895
.float		   -0.67155895 ,    -0.74095113
.float		   -0.02454123 ,     0.99969882	##stage8  group2
.float		   -0.99969882 ,    -0.02454123
.float		   -0.72424708 ,     0.68954054
.float		   -0.68954054 ,    -0.72424708
.float		   -0.40524131 ,     0.91420976
.float		   -0.91420976 ,    -0.40524131
.float		   -0.93299280 ,     0.35989504
.float		   -0.35989504 ,    -0.93299280
.float		   -0.01227154 ,     0.99992470	##stage9  group2
.float		   -0.99992470 ,    -0.01227154
.float		   -0.71573083 ,     0.69837625
.float		   -0.69837625 ,    -0.71573083
.float		   -0.39399204 ,     0.91911385
.float		   -0.91911385 ,    -0.39399204
.float		   -0.92850608 ,     0.37131719
.float		   -0.37131719 ,    -0.92850608
.float		   -0.20711138 ,     0.97831737
.float		   -0.97831737 ,    -0.20711138
.float		   -0.83822471 ,     0.54532499
.float		   -0.54532499 ,    -0.83822471
.float		   -0.56573181 ,     0.82458930
.float		   -0.82458930 ,    -0.56573181
.float		   -0.98310549 ,     0.18303989
.float		   -0.18303989 ,    -0.98310549
.float		   -0.00613588 ,     0.99998118	##stage10  group2
.float		   -0.99998118 ,    -0.00613588
.float		   -0.71143220 ,     0.70275474
.float		   -0.70275474 ,    -0.71143220
.float		   -0.38834505 ,     0.92151404
.float		   -0.92151404 ,    -0.38834505
.float		   -0.92621024 ,     0.37700741
.float		   -0.37700741 ,    -0.92621024
.float		   -0.20110463 ,     0.97956977
.float		   -0.97956977 ,    -0.20110463
.float		   -0.83486287 ,     0.55045797
.float		   -0.55045797 ,    -0.83486287
.float		   -0.56066158 ,     0.82804505
.float		   -0.82804505 ,    -0.56066158
.float		   -0.98196387 ,     0.18906866
.float		   -0.18906866 ,    -0.98196387
.float		   -0.10412163 ,     0.99456457
.float		   -0.99456457 ,    -0.10412163
.float		   -0.77688847 ,     0.62963824
.float		   -0.62963824 ,    -0.77688847
.float		   -0.47679923 ,     0.87901223
.float		   -0.87901223 ,    -0.47679923
.float		   -0.95870347 ,     0.28440754
.float		   -0.28440754 ,    -0.95870347
.float		   -0.29615089 ,     0.95514117
.float		   -0.95514117 ,    -0.29615089
.float		   -0.88479710 ,     0.46597650
.float		   -0.46597650 ,    -0.88479710
.float		   -0.63912444 ,     0.76910334
.float		   -0.76910334 ,    -0.63912444
.float		   -0.99576741 ,     0.09190896
.float		   -0.09190896 ,    -0.99576741
.float		   -0.19509032 ,     0.98078528 ##stage6 group3
.float		   -0.98078528 ,    -0.19509032
.float		   -0.19509032 ,     0.98078528
.float		   -0.98078528 ,    -0.19509032
.float		   -0.09801714 ,     0.99518473	##stage7  group3
.float		   -0.99518473 ,    -0.09801714
.float		   -0.77301045 ,     0.63439328
.float		   -0.63439328 ,    -0.77301045
.float		   -0.04906767 ,     0.99879546	##stage8  group3
.float		   -0.99879546 ,    -0.04906767
.float		   -0.74095113 ,     0.67155895
.float		   -0.67155895 ,    -0.74095113
.float		   -0.42755509 ,     0.90398929
.float		   -0.90398929 ,    -0.42755509
.float		   -0.94154407 ,     0.33688985
.float		   -0.33688985 ,    -0.94154407
.float		   -0.02454123 ,     0.99969882	##stage9  group3
.float		   -0.99969882 ,    -0.02454123
.float		   -0.72424708 ,     0.68954054
.float		   -0.68954054 ,    -0.72424708
.float		   -0.40524131 ,     0.91420976
.float		   -0.91420976 ,    -0.40524131
.float		   -0.93299280 ,     0.35989504
.float		   -0.35989504 ,    -0.93299280
.float		   -0.21910124 ,     0.97570213
.float		   -0.97570213 ,    -0.21910124
.float		   -0.84485357 ,     0.53499762
.float		   -0.53499762 ,    -0.84485357
.float		   -0.57580819 ,     0.81758481
.float		   -0.81758481 ,    -0.57580819
.float		   -0.98527764 ,     0.17096189
.float		   -0.17096189 ,    -0.98527764
.float		   -0.01227154 ,     0.99992470	##stage10  group3
.float		   -0.99992470 ,    -0.01227154
.float		   -0.71573083 ,     0.69837625
.float		   -0.69837625 ,    -0.71573083
.float		   -0.39399204 ,     0.91911385
.float		   -0.91911385 ,    -0.39399204
.float		   -0.92850608 ,     0.37131719
.float		   -0.37131719 ,    -0.92850608
.float		   -0.20711138 ,     0.97831737
.float		   -0.97831737 ,    -0.20711138
.float		   -0.83822471 ,     0.54532499
.float		   -0.54532499 ,    -0.83822471
.float		   -0.56573181 ,     0.82458930
.float		   -0.82458930 ,    -0.56573181
.float		   -0.98310549 ,     0.18303989
.float		   -0.18303989 ,    -0.98310549
.float		   -0.11022221 ,     0.99390697
.float		   -0.99390697 ,    -0.11022221
.float		   -0.78073723 ,     0.62485949
.float		   -0.62485949 ,    -0.78073723
.float		   -0.48218377 ,     0.87607009
.float		   -0.87607009 ,    -0.48218377
.float		   -0.96043052 ,     0.27851969
.float		   -0.27851969 ,    -0.96043052
.float		   -0.30200595 ,     0.95330604
.float		   -0.95330604 ,    -0.30200595
.float		   -0.88763962 ,     0.46053871
.float		   -0.46053871 ,    -0.88763962
.float		   -0.64383154 ,     0.76516727
.float		   -0.76516727 ,    -0.64383154
.float		   -0.99631261 ,     0.08579731
.float		   -0.08579731 ,    -0.99631261
.float		   -0.29028468 ,     0.95694034 ##stage6 group4
.float		   -0.95694034 ,    -0.29028468
.float		   -0.29028468 ,     0.95694034
.float		   -0.95694034 ,    -0.29028468
.float		   -0.14673047 ,     0.98917651	##stage7  group4
.float		   -0.98917651 ,    -0.14673047
.float		   -0.80320753 ,     0.59569930
.float		   -0.59569930 ,    -0.80320753
.float		   -0.07356456 ,     0.99729046	##stage8  group4
.float		   -0.99729046 ,    -0.07356456
.float		   -0.75720885 ,     0.65317284
.float		   -0.65317284 ,    -0.75720885
.float		   -0.44961133 ,     0.89322430
.float		   -0.89322430 ,    -0.44961133
.float		   -0.94952818 ,     0.31368174
.float		   -0.31368174 ,    -0.94952818
.float		   -0.03680722 ,     0.99932238	##stage9  group4
.float		   -0.99932238 ,    -0.03680722
.float		   -0.73265427 ,     0.68060100
.float		   -0.68060100 ,    -0.73265427
.float		   -0.41642956 ,     0.90916798
.float		   -0.90916798 ,    -0.41642956
.float		   -0.93733901 ,     0.34841868
.float		   -0.34841868 ,    -0.93733901
.float		   -0.23105811 ,     0.97293995
.float		   -0.97293995 ,    -0.23105811
.float		   -0.85135519 ,     0.52458968
.float		   -0.52458968 ,    -0.85135519
.float		   -0.58579786 ,     0.81045720
.float		   -0.81045720 ,    -0.58579786
.float		   -0.98730142 ,     0.15885814
.float		   -0.15885814 ,    -0.98730142
.float		   -0.01840673 ,     0.99983058	##stage10  group4
.float		   -0.99983058 ,    -0.01840673
.float		   -0.72000251 ,     0.69397146
.float		   -0.69397146 ,    -0.72000251
.float		   -0.39962420 ,     0.91667906
.float		   -0.91667906 ,    -0.39962420
.float		   -0.93076696 ,     0.36561300
.float		   -0.36561300 ,    -0.93076696
.float		   -0.21311032 ,     0.97702814
.float		   -0.97702814 ,    -0.21311032
.float		   -0.84155498 ,     0.54017147
.float		   -0.54017147 ,    -0.84155498
.float		   -0.57078075 ,     0.82110251
.float		   -0.82110251 ,    -0.57078075
.float		   -0.98421009 ,     0.17700422
.float		   -0.17700422 ,    -0.98421009
.float		   -0.11631863 ,     0.99321195
.float		   -0.99321195 ,    -0.11631863
.float		   -0.78455660 ,     0.62005721
.float		   -0.62005721 ,    -0.78455660
.float		   -0.48755016 ,     0.87309498
.float		   -0.87309498 ,    -0.48755016
.float		   -0.96212140 ,     0.27262136
.float		   -0.27262136 ,    -0.96212140
.float		   -0.30784964 ,     0.95143502
.float		   -0.95143502 ,    -0.30784964
.float		   -0.89044872 ,     0.45508359
.float		   -0.45508359 ,    -0.89044872
.float		   -0.64851440 ,     0.76120239
.float		   -0.76120239 ,    -0.64851440
.float		   -0.99682030 ,     0.07968244
.float		   -0.07968244 ,    -0.99682030
.float		   -0.38268343 ,     0.92387953 ##stage6 group5
.float		   -0.92387953 ,    -0.38268343
.float		   -0.38268343 ,     0.92387953
.float		   -0.92387953 ,    -0.38268343
.float		   -0.19509032 ,     0.98078528	##stage7  group5
.float		   -0.98078528 ,    -0.19509032
.float		   -0.83146961 ,     0.55557023
.float		   -0.55557023 ,    -0.83146961
.float		   -0.09801714 ,     0.99518473	##stage8  group5
.float		   -0.99518473 ,    -0.09801714
.float		   -0.77301045 ,     0.63439328
.float		   -0.63439328 ,    -0.77301045
.float		   -0.47139674 ,     0.88192126
.float		   -0.88192126 ,    -0.47139674
.float		   -0.95694034 ,     0.29028468
.float		   -0.29028468 ,    -0.95694034
.float		   -0.04906767 ,     0.99879546	##stage9  group5
.float		   -0.99879546 ,    -0.04906767
.float		   -0.74095113 ,     0.67155895
.float		   -0.67155895 ,    -0.74095113
.float		   -0.42755509 ,     0.90398929
.float		   -0.90398929 ,    -0.42755509
.float		   -0.94154407 ,     0.33688985
.float		   -0.33688985 ,    -0.94154407
.float		   -0.24298018 ,     0.97003125
.float		   -0.97003125 ,    -0.24298018
.float		   -0.85772861 ,     0.51410274
.float		   -0.51410274 ,    -0.85772861
.float		   -0.59569930 ,     0.80320753
.float		   -0.80320753 ,    -0.59569930
.float		   -0.98917651 ,     0.14673047
.float		   -0.14673047 ,    -0.98917651
.float		   -0.02454123 ,     0.99969882	##stage10  group5
.float		   -0.99969882 ,    -0.02454123
.float		   -0.72424708 ,     0.68954054
.float		   -0.68954054 ,    -0.72424708
.float		   -0.40524131 ,     0.91420976
.float		   -0.91420976 ,    -0.40524131
.float		   -0.93299280 ,     0.35989504
.float		   -0.35989504 ,    -0.93299280
.float		   -0.21910124 ,     0.97570213
.float		   -0.97570213 ,    -0.21910124
.float		   -0.84485357 ,     0.53499762
.float		   -0.53499762 ,    -0.84485357
.float		   -0.57580819 ,     0.81758481
.float		   -0.81758481 ,    -0.57580819
.float		   -0.98527764 ,     0.17096189
.float		   -0.17096189 ,    -0.98527764
.float		   -0.12241068 ,     0.99247953
.float		   -0.99247953 ,    -0.12241068
.float		   -0.78834643 ,     0.61523159
.float		   -0.61523159 ,    -0.78834643
.float		   -0.49289819 ,     0.87008699
.float		   -0.87008699 ,    -0.49289819
.float		   -0.96377607 ,     0.26671276
.float		   -0.26671276 ,    -0.96377607
.float		   -0.31368174 ,     0.94952818
.float		   -0.94952818 ,    -0.31368174
.float		   -0.89322430 ,     0.44961133
.float		   -0.44961133 ,    -0.89322430
.float		   -0.65317284 ,     0.75720885
.float		   -0.75720885 ,    -0.65317284
.float		   -0.99729046 ,     0.07356456
.float		   -0.07356456 ,    -0.99729046
.float		   -0.47139674 ,     0.88192126 ##stage6 group6
.float		   -0.88192126 ,    -0.47139674
.float		   -0.47139674 ,     0.88192126
.float		   -0.88192126 ,    -0.47139674
.float		   -0.24298018 ,     0.97003125	##stage7  group6
.float		   -0.97003125 ,    -0.24298018
.float		   -0.85772861 ,     0.51410274
.float		   -0.51410274 ,    -0.85772861
.float		   -0.12241068 ,     0.99247953	##stage8  group6
.float		   -0.99247953 ,    -0.12241068
.float		   -0.78834643 ,     0.61523159
.float		   -0.61523159 ,    -0.78834643
.float		   -0.49289819 ,     0.87008699
.float		   -0.87008699 ,    -0.49289819
.float		   -0.96377607 ,     0.26671276
.float		   -0.26671276 ,    -0.96377607
.float		   -0.06132074 ,     0.99811811	##stage9  group6
.float		   -0.99811811 ,    -0.06132074
.float		   -0.74913639 ,     0.66241578
.float		   -0.66241578 ,    -0.74913639
.float		   -0.43861624 ,     0.89867447
.float		   -0.89867447 ,    -0.43861624
.float		   -0.94560733 ,     0.32531029
.float		   -0.32531029 ,    -0.94560733
.float		   -0.25486566 ,     0.96697647
.float		   -0.96697647 ,    -0.25486566
.float		   -0.86397286 ,     0.50353838
.float		   -0.50353838 ,    -0.86397286
.float		   -0.60551104 ,     0.79583690
.float		   -0.79583690 ,    -0.60551104
.float		   -0.99090264 ,     0.13458071
.float		   -0.13458071 ,    -0.99090264
.float		   -0.03067480 ,     0.99952942	##stage10  group6
.float		   -0.99952942 ,    -0.03067480
.float		   -0.72846439 ,     0.68508367
.float		   -0.68508367 ,    -0.72846439
.float		   -0.41084317 ,     0.91170603
.float		   -0.91170603 ,    -0.41084317
.float		   -0.93518351 ,     0.35416353
.float		   -0.35416353 ,    -0.93518351
.float		   -0.22508391 ,     0.97433938
.float		   -0.97433938 ,    -0.22508391
.float		   -0.84812034 ,     0.52980362
.float		   -0.52980362 ,    -0.84812034
.float		   -0.58081396 ,     0.81403633
.float		   -0.81403633 ,    -0.58081396
.float		   -0.98630810 ,     0.16491312
.float		   -0.16491312 ,    -0.98630810
.float		   -0.12849811 ,     0.99170975
.float		   -0.99170975 ,    -0.12849811
.float		   -0.79210658 ,     0.61038281
.float		   -0.61038281 ,    -0.79210658
.float		   -0.49822767 ,     0.86704625
.float		   -0.86704625 ,    -0.49822767
.float		   -0.96539444 ,     0.26079412
.float		   -0.26079412 ,    -0.96539444
.float		   -0.31950203 ,     0.94758559
.float		   -0.94758559 ,    -0.31950203
.float		   -0.89596625 ,     0.44412214
.float		   -0.44412214 ,    -0.89596625
.float		   -0.65780669 ,     0.75318680
.float		   -0.75318680 ,    -0.65780669
.float		   -0.99772307 ,     0.06744392
.float		   -0.06744392 ,    -0.99772307
.float		   -0.55557023 ,     0.83146961 ##stage6 group7
.float		   -0.83146961 ,    -0.55557023
.float		   -0.55557023 ,     0.83146961
.float		   -0.83146961 ,    -0.55557023
.float		   -0.29028468 ,     0.95694034	##stage7  group7
.float		   -0.95694034 ,    -0.29028468
.float		   -0.88192126 ,     0.47139674
.float		   -0.47139674 ,    -0.88192126
.float		   -0.14673047 ,     0.98917651	##stage8  group7
.float		   -0.98917651 ,    -0.14673047
.float		   -0.80320753 ,     0.59569930
.float		   -0.59569930 ,    -0.80320753
.float		   -0.51410274 ,     0.85772861
.float		   -0.85772861 ,    -0.51410274
.float		   -0.97003125 ,     0.24298018
.float		   -0.24298018 ,    -0.97003125
.float		   -0.07356456 ,     0.99729046	##stage9  group7
.float		   -0.99729046 ,    -0.07356456
.float		   -0.75720885 ,     0.65317284
.float		   -0.65317284 ,    -0.75720885
.float		   -0.44961133 ,     0.89322430
.float		   -0.89322430 ,    -0.44961133
.float		   -0.94952818 ,     0.31368174
.float		   -0.31368174 ,    -0.94952818
.float		   -0.26671276 ,     0.96377607
.float		   -0.96377607 ,    -0.26671276
.float		   -0.87008699 ,     0.49289819
.float		   -0.49289819 ,    -0.87008699
.float		   -0.61523159 ,     0.78834643
.float		   -0.78834643 ,    -0.61523159
.float		   -0.99247953 ,     0.12241068
.float		   -0.12241068 ,    -0.99247953
.float		   -0.03680722 ,     0.99932238	##stage10  group7
.float		   -0.99932238 ,    -0.03680722
.float		   -0.73265427 ,     0.68060100
.float		   -0.68060100 ,    -0.73265427
.float		   -0.41642956 ,     0.90916798
.float		   -0.90916798 ,    -0.41642956
.float		   -0.93733901 ,     0.34841868
.float		   -0.34841868 ,    -0.93733901
.float		   -0.23105811 ,     0.97293995
.float		   -0.97293995 ,    -0.23105811
.float		   -0.85135519 ,     0.52458968
.float		   -0.52458968 ,    -0.85135519
.float		   -0.58579786 ,     0.81045720
.float		   -0.81045720 ,    -0.58579786
.float		   -0.98730142 ,     0.15885814
.float		   -0.15885814 ,    -0.98730142
.float		   -0.13458071 ,     0.99090264
.float		   -0.99090264 ,    -0.13458071
.float		   -0.79583690 ,     0.60551104
.float		   -0.60551104 ,    -0.79583690
.float		   -0.50353838 ,     0.86397286
.float		   -0.86397286 ,    -0.50353838
.float		   -0.96697647 ,     0.25486566
.float		   -0.25486566 ,    -0.96697647
.float		   -0.32531029 ,     0.94560733
.float		   -0.94560733 ,    -0.32531029
.float		   -0.89867447 ,     0.43861624
.float		   -0.43861624 ,    -0.89867447
.float		   -0.66241578 ,     0.74913639
.float		   -0.74913639 ,    -0.66241578
.float		   -0.99811811 ,     0.06132074
.float		   -0.06132074 ,    -0.99811811
.float		   -0.63439328 ,     0.77301045 ##stage6 group8
.float		   -0.77301045 ,    -0.63439328
.float		   -0.63439328 ,     0.77301045
.float		   -0.77301045 ,    -0.63439328
.float		   -0.33688985 ,     0.94154407	##stage7  group8
.float		   -0.94154407 ,    -0.33688985
.float		   -0.90398929 ,     0.42755509
.float		   -0.42755509 ,    -0.90398929
.float		   -0.17096189 ,     0.98527764	##stage8  group8
.float		   -0.98527764 ,    -0.17096189
.float		   -0.81758481 ,     0.57580819
.float		   -0.57580819 ,    -0.81758481
.float		   -0.53499762 ,     0.84485357
.float		   -0.84485357 ,    -0.53499762
.float		   -0.97570213 ,     0.21910124
.float		   -0.21910124 ,    -0.97570213
.float		   -0.08579731 ,     0.99631261	##stage9  group8
.float		   -0.99631261 ,    -0.08579731
.float		   -0.76516727 ,     0.64383154
.float		   -0.64383154 ,    -0.76516727
.float		   -0.46053871 ,     0.88763962
.float		   -0.88763962 ,    -0.46053871
.float		   -0.95330604 ,     0.30200595
.float		   -0.30200595 ,    -0.95330604
.float		   -0.27851969 ,     0.96043052
.float		   -0.96043052 ,    -0.27851969
.float		   -0.87607009 ,     0.48218377
.float		   -0.48218377 ,    -0.87607009
.float		   -0.62485949 ,     0.78073723
.float		   -0.78073723 ,    -0.62485949
.float		   -0.99390697 ,     0.11022221
.float		   -0.11022221 ,    -0.99390697
.float		   -0.04293826 ,     0.99907773	##stage10  group8
.float		   -0.99907773 ,    -0.04293826
.float		   -0.73681657 ,     0.67609270
.float		   -0.67609270 ,    -0.73681657
.float		   -0.42200027 ,     0.90659570
.float		   -0.90659570 ,    -0.42200027
.float		   -0.93945922 ,     0.34266072
.float		   -0.34266072 ,    -0.93945922
.float		   -0.23702361 ,     0.97150389
.float		   -0.97150389 ,    -0.23702361
.float		   -0.85455799 ,     0.51935599
.float		   -0.51935599 ,    -0.85455799
.float		   -0.59075970 ,     0.80684755
.float		   -0.80684755 ,    -0.59075970
.float		   -0.98825757 ,     0.15279719
.float		   -0.15279719 ,    -0.98825757
.float		   -0.14065824 ,     0.99005821
.float		   -0.99005821 ,    -0.14065824
.float		   -0.79953727 ,     0.60061648
.float		   -0.60061648 ,    -0.79953727
.float		   -0.50883014 ,     0.86086694
.float		   -0.86086694 ,    -0.50883014
.float		   -0.96852209 ,     0.24892761
.float		   -0.24892761 ,    -0.96852209
.float		   -0.33110631 ,     0.94359346
.float		   -0.94359346 ,    -0.33110631
.float		   -0.90134885 ,     0.43309382
.float		   -0.43309382 ,    -0.90134885
.float		   -0.66699992 ,     0.74505779
.float		   -0.74505779 ,    -0.66699992
.float		   -0.99847558 ,     0.05519524
.float		   -0.05519524 ,    -0.99847558
.float		   -0.70710678 ,     0.70710678 ##stage6 group9
.float		   -0.70710678 ,    -0.70710678
.float		   -0.70710678 ,     0.70710678
.float		   -0.70710678 ,    -0.70710678
.float		   -0.38268343 ,     0.92387953	##stage7  group9
.float		   -0.92387953 ,    -0.38268343
.float		   -0.92387953 ,     0.38268343
.float		   -0.38268343 ,    -0.92387953
.float		   -0.19509032 ,     0.98078528	##stage8  group9
.float		   -0.98078528 ,    -0.19509032
.float		   -0.83146961 ,     0.55557023
.float		   -0.55557023 ,    -0.83146961
.float		   -0.55557023 ,     0.83146961
.float		   -0.83146961 ,    -0.55557023
.float		   -0.98078528 ,     0.19509032
.float		   -0.19509032 ,    -0.98078528
.float		   -0.09801714 ,     0.99518473	##stage9  group9
.float		   -0.99518473 ,    -0.09801714
.float		   -0.77301045 ,     0.63439328
.float		   -0.63439328 ,    -0.77301045
.float		   -0.47139674 ,     0.88192126
.float		   -0.88192126 ,    -0.47139674
.float		   -0.95694034 ,     0.29028468
.float		   -0.29028468 ,    -0.95694034
.float		   -0.29028468 ,     0.95694034
.float		   -0.95694034 ,    -0.29028468
.float		   -0.88192126 ,     0.47139674
.float		   -0.47139674 ,    -0.88192126
.float		   -0.63439328 ,     0.77301045
.float		   -0.77301045 ,    -0.63439328
.float		   -0.99518473 ,     0.09801714
.float		   -0.09801714 ,    -0.99518473
.float		   -0.04906767 ,     0.99879546	##stage10  group9
.float		   -0.99879546 ,    -0.04906767
.float		   -0.74095113 ,     0.67155895
.float		   -0.67155895 ,    -0.74095113
.float		   -0.42755509 ,     0.90398929
.float		   -0.90398929 ,    -0.42755509
.float		   -0.94154407 ,     0.33688985
.float		   -0.33688985 ,    -0.94154407
.float		   -0.24298018 ,     0.97003125
.float		   -0.97003125 ,    -0.24298018
.float		   -0.85772861 ,     0.51410274
.float		   -0.51410274 ,    -0.85772861
.float		   -0.59569930 ,     0.80320753
.float		   -0.80320753 ,    -0.59569930
.float		   -0.98917651 ,     0.14673047
.float		   -0.14673047 ,    -0.98917651
.float		   -0.14673047 ,     0.98917651
.float		   -0.98917651 ,    -0.14673047
.float		   -0.80320753 ,     0.59569930
.float		   -0.59569930 ,    -0.80320753
.float		   -0.51410274 ,     0.85772861
.float		   -0.85772861 ,    -0.51410274
.float		   -0.97003125 ,     0.24298018
.float		   -0.24298018 ,    -0.97003125
.float		   -0.33688985 ,     0.94154407
.float		   -0.94154407 ,    -0.33688985
.float		   -0.90398929 ,     0.42755509
.float		   -0.42755509 ,    -0.90398929
.float		   -0.67155895 ,     0.74095113
.float		   -0.74095113 ,    -0.67155895
.float		   -0.99879546 ,     0.04906767
.float		   -0.04906767 ,    -0.99879546
.float		   -0.77301045 ,     0.63439328 ##stage6 group10
.float		   -0.63439328 ,    -0.77301045
.float		   -0.77301045 ,     0.63439328
.float		   -0.63439328 ,    -0.77301045
.float		   -0.42755509 ,     0.90398929	##stage7  group10
.float		   -0.90398929 ,    -0.42755509
.float		   -0.94154407 ,     0.33688985
.float		   -0.33688985 ,    -0.94154407
.float		   -0.21910124 ,     0.97570213	##stage8  group10
.float		   -0.97570213 ,    -0.21910124
.float		   -0.84485357 ,     0.53499762
.float		   -0.53499762 ,    -0.84485357
.float		   -0.57580819 ,     0.81758481
.float		   -0.81758481 ,    -0.57580819
.float		   -0.98527764 ,     0.17096189
.float		   -0.17096189 ,    -0.98527764
.float		   -0.11022221 ,     0.99390697	##stage9  group10
.float		   -0.99390697 ,    -0.11022221
.float		   -0.78073723 ,     0.62485949
.float		   -0.62485949 ,    -0.78073723
.float		   -0.48218377 ,     0.87607009
.float		   -0.87607009 ,    -0.48218377
.float		   -0.96043052 ,     0.27851969
.float		   -0.27851969 ,    -0.96043052
.float		   -0.30200595 ,     0.95330604
.float		   -0.95330604 ,    -0.30200595
.float		   -0.88763962 ,     0.46053871
.float		   -0.46053871 ,    -0.88763962
.float		   -0.64383154 ,     0.76516727
.float		   -0.76516727 ,    -0.64383154
.float		   -0.99631261 ,     0.08579731
.float		   -0.08579731 ,    -0.99631261
.float		   -0.05519524 ,     0.99847558	##stage10  group10
.float		   -0.99847558 ,    -0.05519524
.float		   -0.74505779 ,     0.66699992
.float		   -0.66699992 ,    -0.74505779
.float		   -0.43309382 ,     0.90134885
.float		   -0.90134885 ,    -0.43309382
.float		   -0.94359346 ,     0.33110631
.float		   -0.33110631 ,    -0.94359346
.float		   -0.24892761 ,     0.96852209
.float		   -0.96852209 ,    -0.24892761
.float		   -0.86086694 ,     0.50883014
.float		   -0.50883014 ,    -0.86086694
.float		   -0.60061648 ,     0.79953727
.float		   -0.79953727 ,    -0.60061648
.float		   -0.99005821 ,     0.14065824
.float		   -0.14065824 ,    -0.99005821
.float		   -0.15279719 ,     0.98825757
.float		   -0.98825757 ,    -0.15279719
.float		   -0.80684755 ,     0.59075970
.float		   -0.59075970 ,    -0.80684755
.float		   -0.51935599 ,     0.85455799
.float		   -0.85455799 ,    -0.51935599
.float		   -0.97150389 ,     0.23702361
.float		   -0.23702361 ,    -0.97150389
.float		   -0.34266072 ,     0.93945922
.float		   -0.93945922 ,    -0.34266072
.float		   -0.90659570 ,     0.42200027
.float		   -0.42200027 ,    -0.90659570
.float		   -0.67609270 ,     0.73681657
.float		   -0.73681657 ,    -0.67609270
.float		   -0.99907773 ,     0.04293826
.float		   -0.04293826 ,    -0.99907773
.float		   -0.83146961 ,     0.55557023 ##stage6 group11
.float		   -0.55557023 ,    -0.83146961
.float		   -0.83146961 ,     0.55557023
.float		   -0.55557023 ,    -0.83146961
.float		   -0.47139674 ,     0.88192126	##stage7  group11
.float		   -0.88192126 ,    -0.47139674
.float		   -0.95694034 ,     0.29028468
.float		   -0.29028468 ,    -0.95694034
.float		   -0.24298018 ,     0.97003125	##stage8  group11
.float		   -0.97003125 ,    -0.24298018
.float		   -0.85772861 ,     0.51410274
.float		   -0.51410274 ,    -0.85772861
.float		   -0.59569930 ,     0.80320753
.float		   -0.80320753 ,    -0.59569930
.float		   -0.98917651 ,     0.14673047
.float		   -0.14673047 ,    -0.98917651
.float		   -0.12241068 ,     0.99247953	##stage9  group11
.float		   -0.99247953 ,    -0.12241068
.float		   -0.78834643 ,     0.61523159
.float		   -0.61523159 ,    -0.78834643
.float		   -0.49289819 ,     0.87008699
.float		   -0.87008699 ,    -0.49289819
.float		   -0.96377607 ,     0.26671276
.float		   -0.26671276 ,    -0.96377607
.float		   -0.31368174 ,     0.94952818
.float		   -0.94952818 ,    -0.31368174
.float		   -0.89322430 ,     0.44961133
.float		   -0.44961133 ,    -0.89322430
.float		   -0.65317284 ,     0.75720885
.float		   -0.75720885 ,    -0.65317284
.float		   -0.99729046 ,     0.07356456
.float		   -0.07356456 ,    -0.99729046
.float		   -0.06132074 ,     0.99811811	##stage10  group11
.float		   -0.99811811 ,    -0.06132074
.float		   -0.74913639 ,     0.66241578
.float		   -0.66241578 ,    -0.74913639
.float		   -0.43861624 ,     0.89867447
.float		   -0.89867447 ,    -0.43861624
.float		   -0.94560733 ,     0.32531029
.float		   -0.32531029 ,    -0.94560733
.float		   -0.25486566 ,     0.96697647
.float		   -0.96697647 ,    -0.25486566
.float		   -0.86397286 ,     0.50353838
.float		   -0.50353838 ,    -0.86397286
.float		   -0.60551104 ,     0.79583690
.float		   -0.79583690 ,    -0.60551104
.float		   -0.99090264 ,     0.13458071
.float		   -0.13458071 ,    -0.99090264
.float		   -0.15885814 ,     0.98730142
.float		   -0.98730142 ,    -0.15885814
.float		   -0.81045720 ,     0.58579786
.float		   -0.58579786 ,    -0.81045720
.float		   -0.52458968 ,     0.85135519
.float		   -0.85135519 ,    -0.52458968
.float		   -0.97293995 ,     0.23105811
.float		   -0.23105811 ,    -0.97293995
.float		   -0.34841868 ,     0.93733901
.float		   -0.93733901 ,    -0.34841868
.float		   -0.90916798 ,     0.41642956
.float		   -0.41642956 ,    -0.90916798
.float		   -0.68060100 ,     0.73265427
.float		   -0.73265427 ,    -0.68060100
.float		   -0.99932238 ,     0.03680722
.float		   -0.03680722 ,    -0.99932238
.float		   -0.88192126 ,     0.47139674 ##stage6 group12
.float		   -0.47139674 ,    -0.88192126
.float		   -0.88192126 ,     0.47139674
.float		   -0.47139674 ,    -0.88192126
.float		   -0.51410274 ,     0.85772861	##stage7  group12
.float		   -0.85772861 ,    -0.51410274
.float		   -0.97003125 ,     0.24298018
.float		   -0.24298018 ,    -0.97003125
.float		   -0.26671276 ,     0.96377607	##stage8  group12
.float		   -0.96377607 ,    -0.26671276
.float		   -0.87008699 ,     0.49289819
.float		   -0.49289819 ,    -0.87008699
.float		   -0.61523159 ,     0.78834643
.float		   -0.78834643 ,    -0.61523159
.float		   -0.99247953 ,     0.12241068
.float		   -0.12241068 ,    -0.99247953
.float		   -0.13458071 ,     0.99090264	##stage9  group12
.float		   -0.99090264 ,    -0.13458071
.float		   -0.79583690 ,     0.60551104
.float		   -0.60551104 ,    -0.79583690
.float		   -0.50353838 ,     0.86397286
.float		   -0.86397286 ,    -0.50353838
.float		   -0.96697647 ,     0.25486566
.float		   -0.25486566 ,    -0.96697647
.float		   -0.32531029 ,     0.94560733
.float		   -0.94560733 ,    -0.32531029
.float		   -0.89867447 ,     0.43861624
.float		   -0.43861624 ,    -0.89867447
.float		   -0.66241578 ,     0.74913639
.float		   -0.74913639 ,    -0.66241578
.float		   -0.99811811 ,     0.06132074
.float		   -0.06132074 ,    -0.99811811
.float		   -0.06744392 ,     0.99772307	##stage10  group12
.float		   -0.99772307 ,    -0.06744392
.float		   -0.75318680 ,     0.65780669
.float		   -0.65780669 ,    -0.75318680
.float		   -0.44412214 ,     0.89596625
.float		   -0.89596625 ,    -0.44412214
.float		   -0.94758559 ,     0.31950203
.float		   -0.31950203 ,    -0.94758559
.float		   -0.26079412 ,     0.96539444
.float		   -0.96539444 ,    -0.26079412
.float		   -0.86704625 ,     0.49822767
.float		   -0.49822767 ,    -0.86704625
.float		   -0.61038281 ,     0.79210658
.float		   -0.79210658 ,    -0.61038281
.float		   -0.99170975 ,     0.12849811
.float		   -0.12849811 ,    -0.99170975
.float		   -0.16491312 ,     0.98630810
.float		   -0.98630810 ,    -0.16491312
.float		   -0.81403633 ,     0.58081396
.float		   -0.58081396 ,    -0.81403633
.float		   -0.52980362 ,     0.84812034
.float		   -0.84812034 ,    -0.52980362
.float		   -0.97433938 ,     0.22508391
.float		   -0.22508391 ,    -0.97433938
.float		   -0.35416353 ,     0.93518351
.float		   -0.93518351 ,    -0.35416353
.float		   -0.91170603 ,     0.41084317
.float		   -0.41084317 ,    -0.91170603
.float		   -0.68508367 ,     0.72846439
.float		   -0.72846439 ,    -0.68508367
.float		   -0.99952942 ,     0.03067480
.float		   -0.03067480 ,    -0.99952942
.float		   -0.92387953 ,     0.38268343 ##stage6 group13
.float		   -0.38268343 ,    -0.92387953
.float		   -0.92387953 ,     0.38268343
.float		   -0.38268343 ,    -0.92387953
.float		   -0.55557023 ,     0.83146961	##stage7  group13
.float		   -0.83146961 ,    -0.55557023
.float		   -0.98078528 ,     0.19509032
.float		   -0.19509032 ,    -0.98078528
.float		   -0.29028468 ,     0.95694034	##stage8  group13
.float		   -0.95694034 ,    -0.29028468
.float		   -0.88192126 ,     0.47139674
.float		   -0.47139674 ,    -0.88192126
.float		   -0.63439328 ,     0.77301045
.float		   -0.77301045 ,    -0.63439328
.float		   -0.99518473 ,     0.09801714
.float		   -0.09801714 ,    -0.99518473
.float		   -0.14673047 ,     0.98917651	##stage9  group13
.float		   -0.98917651 ,    -0.14673047
.float		   -0.80320753 ,     0.59569930
.float		   -0.59569930 ,    -0.80320753
.float		   -0.51410274 ,     0.85772861
.float		   -0.85772861 ,    -0.51410274
.float		   -0.97003125 ,     0.24298018
.float		   -0.24298018 ,    -0.97003125
.float		   -0.33688985 ,     0.94154407
.float		   -0.94154407 ,    -0.33688985
.float		   -0.90398929 ,     0.42755509
.float		   -0.42755509 ,    -0.90398929
.float		   -0.67155895 ,     0.74095113
.float		   -0.74095113 ,    -0.67155895
.float		   -0.99879546 ,     0.04906767
.float		   -0.04906767 ,    -0.99879546
.float		   -0.07356456 ,     0.99729046	##stage10  group13
.float		   -0.99729046 ,    -0.07356456
.float		   -0.75720885 ,     0.65317284
.float		   -0.65317284 ,    -0.75720885
.float		   -0.44961133 ,     0.89322430
.float		   -0.89322430 ,    -0.44961133
.float		   -0.94952818 ,     0.31368174
.float		   -0.31368174 ,    -0.94952818
.float		   -0.26671276 ,     0.96377607
.float		   -0.96377607 ,    -0.26671276
.float		   -0.87008699 ,     0.49289819
.float		   -0.49289819 ,    -0.87008699
.float		   -0.61523159 ,     0.78834643
.float		   -0.78834643 ,    -0.61523159
.float		   -0.99247953 ,     0.12241068
.float		   -0.12241068 ,    -0.99247953
.float		   -0.17096189 ,     0.98527764
.float		   -0.98527764 ,    -0.17096189
.float		   -0.81758481 ,     0.57580819
.float		   -0.57580819 ,    -0.81758481
.float		   -0.53499762 ,     0.84485357
.float		   -0.84485357 ,    -0.53499762
.float		   -0.97570213 ,     0.21910124
.float		   -0.21910124 ,    -0.97570213
.float		   -0.35989504 ,     0.93299280
.float		   -0.93299280 ,    -0.35989504
.float		   -0.91420976 ,     0.40524131
.float		   -0.40524131 ,    -0.91420976
.float		   -0.68954054 ,     0.72424708
.float		   -0.72424708 ,    -0.68954054
.float		   -0.99969882 ,     0.02454123
.float		   -0.02454123 ,    -0.99969882
.float		   -0.95694034 ,     0.29028468 ##stage6 group14
.float		   -0.29028468 ,    -0.95694034
.float		   -0.95694034 ,     0.29028468
.float		   -0.29028468 ,    -0.95694034
.float		   -0.59569930 ,     0.80320753	##stage7  group14
.float		   -0.80320753 ,    -0.59569930
.float		   -0.98917651 ,     0.14673047
.float		   -0.14673047 ,    -0.98917651
.float		   -0.31368174 ,     0.94952818	##stage8  group14
.float		   -0.94952818 ,    -0.31368174
.float		   -0.89322430 ,     0.44961133
.float		   -0.44961133 ,    -0.89322430
.float		   -0.65317284 ,     0.75720885
.float		   -0.75720885 ,    -0.65317284
.float		   -0.99729046 ,     0.07356456
.float		   -0.07356456 ,    -0.99729046
.float		   -0.15885814 ,     0.98730142	##stage9  group14
.float		   -0.98730142 ,    -0.15885814
.float		   -0.81045720 ,     0.58579786
.float		   -0.58579786 ,    -0.81045720
.float		   -0.52458968 ,     0.85135519
.float		   -0.85135519 ,    -0.52458968
.float		   -0.97293995 ,     0.23105811
.float		   -0.23105811 ,    -0.97293995
.float		   -0.34841868 ,     0.93733901
.float		   -0.93733901 ,    -0.34841868
.float		   -0.90916798 ,     0.41642956
.float		   -0.41642956 ,    -0.90916798
.float		   -0.68060100 ,     0.73265427
.float		   -0.73265427 ,    -0.68060100
.float		   -0.99932238 ,     0.03680722
.float		   -0.03680722 ,    -0.99932238
.float		   -0.07968244 ,     0.99682030	##stage10  group14
.float		   -0.99682030 ,    -0.07968244
.float		   -0.76120239 ,     0.64851440
.float		   -0.64851440 ,    -0.76120239
.float		   -0.45508359 ,     0.89044872
.float		   -0.89044872 ,    -0.45508359
.float		   -0.95143502 ,     0.30784964
.float		   -0.30784964 ,    -0.95143502
.float		   -0.27262136 ,     0.96212140
.float		   -0.96212140 ,    -0.27262136
.float		   -0.87309498 ,     0.48755016
.float		   -0.48755016 ,    -0.87309498
.float		   -0.62005721 ,     0.78455660
.float		   -0.78455660 ,    -0.62005721
.float		   -0.99321195 ,     0.11631863
.float		   -0.11631863 ,    -0.99321195
.float		   -0.17700422 ,     0.98421009
.float		   -0.98421009 ,    -0.17700422
.float		   -0.82110251 ,     0.57078075
.float		   -0.57078075 ,    -0.82110251
.float		   -0.54017147 ,     0.84155498
.float		   -0.84155498 ,    -0.54017147
.float		   -0.97702814 ,     0.21311032
.float		   -0.21311032 ,    -0.97702814
.float		   -0.36561300 ,     0.93076696
.float		   -0.93076696 ,    -0.36561300
.float		   -0.91667906 ,     0.39962420
.float		   -0.39962420 ,    -0.91667906
.float		   -0.69397146 ,     0.72000251
.float		   -0.72000251 ,    -0.69397146
.float		   -0.99983058 ,     0.01840673
.float		   -0.01840673 ,    -0.99983058
.float		   -0.98078528 ,     0.19509032 ##stage6 group15
.float		   -0.19509032 ,    -0.98078528
.float		   -0.98078528 ,     0.19509032
.float		   -0.19509032 ,    -0.98078528
.float		   -0.63439328 ,     0.77301045	##stage7  group15
.float		   -0.77301045 ,    -0.63439328
.float		   -0.99518473 ,     0.09801714
.float		   -0.09801714 ,    -0.99518473
.float		   -0.33688985 ,     0.94154407	##stage8  group15
.float		   -0.94154407 ,    -0.33688985
.float		   -0.90398929 ,     0.42755509
.float		   -0.42755509 ,    -0.90398929
.float		   -0.67155895 ,     0.74095113
.float		   -0.74095113 ,    -0.67155895
.float		   -0.99879546 ,     0.04906767
.float		   -0.04906767 ,    -0.99879546
.float		   -0.17096189 ,     0.98527764	##stage9  group15
.float		   -0.98527764 ,    -0.17096189
.float		   -0.81758481 ,     0.57580819
.float		   -0.57580819 ,    -0.81758481
.float		   -0.53499762 ,     0.84485357
.float		   -0.84485357 ,    -0.53499762
.float		   -0.97570213 ,     0.21910124
.float		   -0.21910124 ,    -0.97570213
.float		   -0.35989504 ,     0.93299280
.float		   -0.93299280 ,    -0.35989504
.float		   -0.91420976 ,     0.40524131
.float		   -0.40524131 ,    -0.91420976
.float		   -0.68954054 ,     0.72424708
.float		   -0.72424708 ,    -0.68954054
.float		   -0.99969882 ,     0.02454123
.float		   -0.02454123 ,    -0.99969882
.float		   -0.08579731 ,     0.99631261	##stage10  group15
.float		   -0.99631261 ,    -0.08579731
.float		   -0.76516727 ,     0.64383154
.float		   -0.64383154 ,    -0.76516727
.float		   -0.46053871 ,     0.88763962
.float		   -0.88763962 ,    -0.46053871
.float		   -0.95330604 ,     0.30200595
.float		   -0.30200595 ,    -0.95330604
.float		   -0.27851969 ,     0.96043052
.float		   -0.96043052 ,    -0.27851969
.float		   -0.87607009 ,     0.48218377
.float		   -0.48218377 ,    -0.87607009
.float		   -0.62485949 ,     0.78073723
.float		   -0.78073723 ,    -0.62485949
.float		   -0.99390697 ,     0.11022221
.float		   -0.11022221 ,    -0.99390697
.float		   -0.18303989 ,     0.98310549
.float		   -0.98310549 ,    -0.18303989
.float		   -0.82458930 ,     0.56573181
.float		   -0.56573181 ,    -0.82458930
.float		   -0.54532499 ,     0.83822471
.float		   -0.83822471 ,    -0.54532499
.float		   -0.97831737 ,     0.20711138
.float		   -0.20711138 ,    -0.97831737
.float		   -0.37131719 ,     0.92850608
.float		   -0.92850608 ,    -0.37131719
.float		   -0.91911385 ,     0.39399204
.float		   -0.39399204 ,    -0.91911385
.float		   -0.69837625 ,     0.71573083
.float		   -0.71573083 ,    -0.69837625
.float		   -0.99992470 ,     0.01227154
.float		   -0.01227154 ,    -0.99992470
.float		   -0.99518473 ,     0.09801714 ##stage6 group16
.float		   -0.09801714 ,    -0.99518473
.float		   -0.99518473 ,     0.09801714
.float		   -0.09801714 ,    -0.99518473
.float		   -0.67155895 ,     0.74095113	##stage7  group16
.float		   -0.74095113 ,    -0.67155895
.float		   -0.99879546 ,     0.04906767
.float		   -0.04906767 ,    -0.99879546
.float		   -0.35989504 ,     0.93299280	##stage8  group16
.float		   -0.93299280 ,    -0.35989504
.float		   -0.91420976 ,     0.40524131
.float		   -0.40524131 ,    -0.91420976
.float		   -0.68954054 ,     0.72424708
.float		   -0.72424708 ,    -0.68954054
.float		   -0.99969882 ,     0.02454123
.float		   -0.02454123 ,    -0.99969882
.float		   -0.18303989 ,     0.98310549	##stage9  group16
.float		   -0.98310549 ,    -0.18303989
.float		   -0.82458930 ,     0.56573181
.float		   -0.56573181 ,    -0.82458930
.float		   -0.54532499 ,     0.83822471
.float		   -0.83822471 ,    -0.54532499
.float		   -0.97831737 ,     0.20711138
.float		   -0.20711138 ,    -0.97831737
.float		   -0.37131719 ,     0.92850608
.float		   -0.92850608 ,    -0.37131719
.float		   -0.91911385 ,     0.39399204
.float		   -0.39399204 ,    -0.91911385
.float		   -0.69837625 ,     0.71573083
.float		   -0.71573083 ,    -0.69837625
.float		   -0.99992470 ,     0.01227154
.float		   -0.01227154 ,    -0.99992470
.float		   -0.09190896 ,     0.99576741	##stage10  group16
.float		   -0.99576741 ,    -0.09190896
.float		   -0.76910334 ,     0.63912444
.float		   -0.63912444 ,    -0.76910334
.float		   -0.46597650 ,     0.88479710
.float		   -0.88479710 ,    -0.46597650
.float		   -0.95514117 ,     0.29615089
.float		   -0.29615089 ,    -0.95514117
.float		   -0.28440754 ,     0.95870347
.float		   -0.95870347 ,    -0.28440754
.float		   -0.87901223 ,     0.47679923
.float		   -0.47679923 ,    -0.87901223
.float		   -0.62963824 ,     0.77688847
.float		   -0.77688847 ,    -0.62963824
.float		   -0.99456457 ,     0.10412163
.float		   -0.10412163 ,    -0.99456457
.float		   -0.18906866 ,     0.98196387
.float		   -0.98196387 ,    -0.18906866
.float		   -0.82804505 ,     0.56066158
.float		   -0.56066158 ,    -0.82804505
.float		   -0.55045797 ,     0.83486287
.float		   -0.83486287 ,    -0.55045797
.float		   -0.97956977 ,     0.20110463
.float		   -0.20110463 ,    -0.97956977
.float		   -0.37700741 ,     0.92621024
.float		   -0.92621024 ,    -0.37700741
.float		   -0.92151404 ,     0.38834505
.float		   -0.38834505 ,    -0.92151404
.float		   -0.70275474 ,     0.71143220
.float		   -0.71143220 ,    -0.70275474
.float		   -0.99998118 ,     0.00613588
.float		   -0.00613588 ,    -0.99998118

fft_dout:
.float		c415f7c6 , 44d446e0	##1
.float		c54301dc , c481c408	##2
.float		c3d013e1 , 4580feb6	##3
.float		44fc1467 , c49e6155	##4
.float		c5589824 , c597afc4	##5
.float		c5173b27 , c3818616	##6
.float		c5166d53 , c4ae976b	##7
.float		449c2a01 , 453735e9	##8
.float		458b6efa , 452984fb	##9
.float		c2f9095e , 45b53a94	##10
.float		44bc2d31 , c4c7d0dc	##11
.float		4417c7e3 , 4453bdb3	##12
.float		c5053544 , 452835d4	##13
.float		449e088c , 4588f763	##14
.float		446e0f88 , 45129711	##15
.float		c5805628 , c45eb05a	##16
.float		c497a573 , c56341a6	##17
.float		4577639d , 45054302	##18
.float		42f51314 , c4c2064c	##19
.float		43881f28 , 44915d15	##20
.float		c500c8e0 , 455bb010	##21
.float		c43c22ae , c4f52308	##22
.float		c49e1a99 , c4803773	##23
.float		450af79c , c59c8953	##24
.float		459932cf , c4cb5b54	##25
.float		4595e744 , 45956c10	##26
.float		c5701e7a , 44daf349	##27
.float		44b431a8 , 437e23c3	##28
.float		45289c22 , c53059be	##29
.float		44a358e7 , 449833ac	##30
.float		c31fe5f9 , c4541245	##31
.float		44dfb7f4 , c51218f8	##32
.float		43e73a86 , c4e1921e	##33
.float		44aed7bb , c352ee98	##34
.float		43763496 , 45339d77	##35
.float		4534f872 , 451960f2	##36
.float		4495bd10 , 450daa60	##37
.float		447ea4a2 , c523df51	##38
.float		44d47b87 , 45699797	##39
.float		c4984bec , c51b2714	##40
.float		430d9e2f , c523d185	##41
.float		42c83729 , c4bc8104	##42
.float		4514c0e1 , c3dc18d3	##43
.float		4413df94 , c514c662	##44
.float		4554dcba , 45610c03	##45
.float		4424d6c9 , 44407eed	##46
.float		c6034cbe , c53653fb	##47
.float		c501f973 , 44f8a6e6	##48
.float		c526cadb , c54394e7	##49
.float		4483191d , 44837cd8	##50
.float		44c150c0 , c4deaa61	##51
.float		45069c3a , 44a0fa49	##52
.float		c4cfe77c , c5483af1	##53
.float		c591c09c , c4c3e672	##54
.float		c4e0f343 , 44e5f2f5	##55
.float		c5b3c3e5 , 4521a63e	##56
.float		45353c9a , 45172dac	##57
.float		43b9a2ac , 44958bbc	##58
.float		c4031fba , c50551a0	##59
.float		45a40115 , c382c3fb	##60
.float		c2fb153a , c4f870fe	##61
.float		c45aec32 , c52cc388	##62
.float		424500e3 , 43d189f3	##63
.float		c5a9893c , 455713ec	##64
.float		448b7f07 , c4f92eb5	##65
.float		c4b63b75 , 45275cff	##66
.float		444a5ca4 , 4498dbf9	##67
.float		c511b211 , c4b78d60	##68
.float		43a57e81 , 455fe40b	##69
.float		c55feb24 , c4f26bd9	##70
.float		45871a6f , c492ab62	##71
.float		4233c6f4 , 446796ce	##72
.float		43bfbc33 , c3a9af24	##73
.float		44976c10 , 44acdca4	##74
.float		c4c5dd24 , c414c3a1	##75
.float		447e0d33 , 4591653f	##76
.float		44bde265 , 44212904	##77
.float		c467f36a , 446621bd	##78
.float		c5526470 , 45336cdd	##79
.float		c571a323 , 44b6e8c9	##80
.float		c32011a9 , 4551b6ea	##81
.float		454065c5 , c4b3dd9e	##82
.float		c4826f60 , 440c2a1d	##83
.float		c50d14a1 , 4354923f	##84
.float		c545a6da , 45444ef5	##85
.float		c40b6f46 , 453fac70	##86
.float		41b94d53 , c49f1fa5	##87
.float		45bdb98e , 42ce9b65	##88
.float		c3211a6c , 453a3e70	##89
.float		45ab13b7 , 44db85df	##90
.float		455db926 , 450efcad	##91
.float		c487ee4e , c3a20d5a	##92
.float		c56dace0 , c50a8b3d	##93
.float		431c780b , 44df5178	##94
.float		447d7861 , c58dafb0	##95
.float		c4416eb2 , c53393fc	##96
.float		c3e574b4 , c472ad9e	##97
.float		c4683f6b , 4506297a	##98
.float		c564ece5 , c532bc40	##99
.float		c507488c , c40004d8	##100
.float		4429939d , 44f80581	##101
.float		c544cec4 , c220874c	##102
.float		44f4bdd1 , c5ed1d97	##103
.float		c51e8948 , c48d0de6	##104
.float		c324420f , c4023f54	##105
.float		4561aa94 , 4520884c	##106
.float		c52d518f , 451026e7	##107
.float		c4c19f28 , c49108aa	##108
.float		43e1b6de , c4221e6a	##109
.float		4412d029 , 43e19802	##110
.float		c49673ca , c3bc9834	##111
.float		43dce985 , 452da898	##112
.float		43b65a53 , 45a8898b	##113
.float		43fb2023 , c4f2ffce	##114
.float		c43551a4 , 440a40c2	##115
.float		c3ab54df , c3a7604c	##116
.float		44bb069b , c464cd0e	##117
.float		44896d85 , c51b75ce	##118
.float		44cdcb2a , c3a0557a	##119
.float		c587e061 , c5b943a5	##120
.float		c4709a9b , c42b3fa5	##121
.float		4540763e , 44d419a1	##122
.float		45566896 , 443cd33c	##123
.float		c4b95bd1 , c508256d	##124
.float		c2a07e5f , c46d36ac	##125
.float		45810bbe , 44bd1b0b	##126
.float		c528e8fd , 44f67e44	##127
.float		44f983c6 , 43925dc0	##128
.float		44135ef5 , 45000980	##129
.float		456eb21f , 443fd008	##130
.float		c5958a6d , 44d1a9ea	##131
.float		c322304f , 44a82d18	##132
.float		c2bca16f , c4d2164d	##133
.float		c50cf34c , c4b44ece	##134
.float		452ce56b , 441c439d	##135
.float		c5ade4bb , c442d0ac	##136
.float		4597c237 , 44d46e17	##137
.float		45bb6631 , c5031c0c	##138
.float		454230cf , 44617106	##139
.float		4489a7e9 , 44910572	##140
.float		c4ac1ffa , c59a20c4	##141
.float		c530e885 , 44cfb437	##142
.float		44018afe , 449efa1d	##143
.float		c52c7d54 , 44d79172	##144
.float		441f76bf , 438eba44	##145
.float		c58b4629 , 451cc810	##146
.float		c544e21f , c51c4353	##147
.float		44d324c8 , c4d91a6c	##148
.float		4390eb20 , c5e45325	##149
.float		c509a955 , c410801c	##150
.float		4527081e , 458cad2c	##151
.float		45053e11 , 4424397b	##152
.float		45470096 , c2ae5729	##153
.float		c3d57fac , 4580975b	##154
.float		44ef74c1 , 459a2c98	##155
.float		44fd7586 , c4d50855	##156
.float		c4f11aa1 , 45140ae9	##157
.float		c492e981 , c5304d31	##158
.float		c5c0728a , 44eedb89	##159
.float		c52a96ba , 4591333d	##160
.float		450b776f , c528efa2	##161
.float		c4d4b8e9 , 43d352df	##162
.float		c4e87abc , c50bd6bc	##163
.float		455c2965 , 441d693b	##164
.float		c5605caf , c4b53ea9	##165
.float		44d2fe59 , c5260305	##166
.float		4363fd59 , 45b77800	##167
.float		c4ea6e24 , 45043401	##168
.float		44347c9d , c50a198d	##169
.float		c58e2577 , 44d80400	##170
.float		44fac52b , c503ec09	##171
.float		4509f7c1 , c34c3dc0	##172
.float		c4007790 , c563ff7f	##173
.float		451e173c , c4f5fd86	##174
.float		c1f21ad5 , c4c632c2	##175
.float		4561e4d4 , c4f4e3a6	##176
.float		c5285f5f , 44ce04c6	##177
.float		44abf483 , c3f62eeb	##178
.float		4575f112 , 449d0e77	##179
.float		c4ebfa80 , 45b5af90	##180
.float		c584f97f , c5143923	##181
.float		4526cb64 , 458547aa	##182
.float		451c0d0a , c2538dc8	##183
.float		c499862b , 446d1194	##184
.float		c40fcace , c40464ea	##185
.float		c530f6e2 , 43dcb9c6	##186
.float		c54eb2d0 , 4563e870	##187
.float		c5010958 , 4393c150	##188
.float		45e3816f , 4586b8b0	##189
.float		c55861cd , 450b1699	##190
.float		4372c521 , c585a819	##191
.float		c250a481 , 44b12065	##192
.float		44e72d9c , 423d3c62	##193
.float		c42272cd , c38de1a6	##194
.float		c59190b1 , 42655a32	##195
.float		4548a070 , c59ea9d2	##196
.float		44c3a189 , c4360697	##197
.float		44d0dff0 , c50518b5	##198
.float		45260d4a , 451da293	##199
.float		4484c88c , c5200119	##200
.float		c529b65f , 45969f6a	##201
.float		c579e185 , 453b0a77	##202
.float		c36fb29b , 43370930	##203
.float		c494a6c6 , c5b1d21d	##204
.float		4498c2f5 , c4bb92d4	##205
.float		452efb3a , c52369b3	##206
.float		451dbda4 , c51cbea1	##207
.float		c479ac68 , 45d96654	##208
.float		c30489f3 , c291a9a6	##209
.float		c4973dba , c4733658	##210
.float		c51c60eb , c42866d4	##211
.float		c58514f8 , c4e4d58e	##212
.float		c509b5c0 , 454dbad3	##213
.float		c5288636 , 43929560	##214
.float		441e5b99 , c5a66219	##215
.float		c377f590 , 454c56bd	##216
.float		c5a893c4 , c4196a85	##217
.float		c501f658 , c3ce2019	##218
.float		c49048d7 , c48e824c	##219
.float		45841430 , 44cb9673	##220
.float		4305d0a9 , c53a068e	##221
.float		44807a78 , c48a7071	##222
.float		4309dcf7 , 44e05f1b	##223
.float		c4fd4a68 , 459c01a3	##224
.float		4538ee02 , c13706cf	##225
.float		c456de7f , c4ed07c0	##226
.float		45555f26 , 448e137a	##227
.float		4546a45c , 43de3d76	##228
.float		c43aa272 , c48648bd	##229
.float		4598ba03 , 45b774ae	##230
.float		c54d6ce5 , 45803282	##231
.float		c3d0002e , c5895fce	##232
.float		451e98cc , 43333bef	##233
.float		c5567da9 , 44d91c6b	##234
.float		44381fc8 , 45b7799a	##235
.float		c42ad71a , c4c6c115	##236
.float		43d979a9 , c47f8847	##237
.float		45b2b5bb , c47ce61a	##238
.float		45583864 , c40dbe3c	##239
.float		458957ae , c3f39318	##240
.float		c56193d5 , 4516a190	##241
.float		c560ae5e , c52ab53e	##242
.float		c524e0eb , 4507da07	##243
.float		456615db , c28d9ad1	##244
.float		44b83c2e , 4516f333	##245
.float		44e1a397 , 448a3832	##246
.float		45b4a1cd , c52faf14	##247
.float		4409d4da , c3f409b5	##248
.float		450defaf , c3742818	##249
.float		c4cf9945 , c5650957	##250
.float		4343c4e9 , 459c8e35	##251
.float		c49f322d , c587b62a	##252
.float		c4bd8905 , 453d1e78	##253
.float		c38708e4 , 451b145e	##254
.float		c56dd208 , c504d92f	##255
.float		c56b1993 , 44ab4578	##256
.float		440eaff9 , c333f51e	##257
.float		44669b7b , c467b5c3	##258
.float		43d901ae , 45363532	##259
.float		4568c2ab , 44b16555	##260
.float		44a9cf8c , 453f07fa	##261
.float		c4ba1d25 , 4442e838	##262
.float		c4fb4f5d , 4514084b	##263
.float		45a799a6 , 43eb307a	##264
.float		440d0c68 , 441435e4	##265
.float		c38336df , c45d0312	##266
.float		45e41e1b , c540cf3d	##267
.float		c45bcbee , c45e25e5	##268
.float		43bfd2d1 , 44ec9356	##269
.float		44d05977 , 44ab63c4	##270
.float		42c06981 , c545bbea	##271
.float		c4f6631f , 45109fe4	##272
.float		c4b2b0d9 , c4db2cf4	##273
.float		441f83c7 , 44b53835	##274
.float		c4843ff7 , c4cf49e2	##275
.float		c4c4feb0 , 44c4041d	##276
.float		4535fbb0 , 4485461b	##277
.float		455445ef , c4e5cfd8	##278
.float		453dca29 , c56f9f44	##279
.float		c549d4d3 , 445e5199	##280
.float		c5792614 , 4570fb8a	##281
.float		c4a8ad3c , c407703c	##282
.float		44f91da0 , 44efecb2	##283
.float		c4903504 , c3cbe61a	##284
.float		c526419c , 452e2132	##285
.float		c4859cf5 , c537c9f8	##286
.float		c589a024 , c4fd3f07	##287
.float		c4c9319b , 4487ce28	##288
.float		c58d1875 , c535b95d	##289
.float		452ca7a8 , 44e6469c	##290
.float		440a5cf8 , c5156ffa	##291
.float		c4d5b842 , 4549c3b3	##292
.float		45250a85 , 44b5ab6e	##293
.float		c4f96c8c , 452f4082	##294
.float		44e6715b , 44ee30f6	##295
.float		c3e88bff , 4433bf87	##296
.float		4500b74b , c49a7048	##297
.float		44521979 , c41a0ed8	##298
.float		c46c3a04 , c4c65135	##299
.float		c59755d6 , 4393deb3	##300
.float		c451bb52 , 4207d30a	##301
.float		c3a1a899 , c5908570	##302
.float		4562fc6e , c5027920	##303
.float		45bbb424 , 44dd0505	##304
.float		44301812 , c482a506	##305
.float		c494e533 , 448cd92a	##306
.float		c55189b6 , c522c24b	##307
.float		c511113b , c3c020cd	##308
.float		448590af , 44d57d86	##309
.float		45db5daa , c4870417	##310
.float		c5883e88 , 44f0b013	##311
.float		c5251926 , c4a31898	##312
.float		c5051dbd , c4145f50	##313
.float		457a159d , 44cb3fec	##314
.float		44a965f7 , c53b4f5b	##315
.float		c4b27e90 , c4b48906	##316
.float		c502b091 , c43a2758	##317
.float		453fc525 , c409ffd0	##318
.float		459bb121 , c54fa6c8	##319
.float		c56e8b40 , c5c1c277	##320
.float		44c87ea4 , c547caab	##321
.float		c5578567 , 432a9df2	##322
.float		c2931ab2 , 44c95a5f	##323
.float		44f3384b , c580a539	##324
.float		c1cd9786 , c4768c12	##325
.float		c506ed09 , 42fdde38	##326
.float		c5c5b898 , 44d29f53	##327
.float		c43c023a , c48abbd1	##328
.float		45801a99 , c4abca12	##329
.float		450af40c , 44c92371	##330
.float		4302230b , c5224f20	##331
.float		4417ce01 , c3c99817	##332
.float		c52e64ee , 45422910	##333
.float		c5101640 , c54d1499	##334
.float		453f2b36 , c534e2b4	##335
.float		c53e1efb , c39f18ac	##336
.float		c4d8cc3d , c292c9fd	##337
.float		c5c5ff3b , 450edc0f	##338
.float		c525cf4a , c4ff10b3	##339
.float		43fee292 , 450c0ce4	##340
.float		c5a5e9a4 , 453f9057	##341
.float		448c73ec , c5258502	##342
.float		45063430 , c4abc822	##343
.float		c5a6a664 , c44bdefd	##344
.float		c4873e3a , c4c551dd	##345
.float		4521fab5 , c53d0550	##346
.float		c53a62ef , 4577623b	##347
.float		c4684dfd , c4bbebd0	##348
.float		c553d79e , c500a66d	##349
.float		445813ac , c4daa424	##350
.float		4456ac3f , c4906094	##351
.float		c3907856 , 44b0fa52	##352
.float		c41f9f48 , c5562fb7	##353
.float		4590f0ab , c3037017	##354
.float		c4172983 , c55a5c6e	##355
.float		440910fa , 45209b62	##356
.float		c4a74288 , c5579e07	##357
.float		c4efb4fc , c56f7f5d	##358
.float		c4351ee1 , c5aef078	##359
.float		44c3a4b5 , 44e5904c	##360
.float		c4e7cf67 , c4857978	##361
.float		45830005 , 458d1bb1	##362
.float		c47abb95 , c434e805	##363
.float		45745e75 , c56bf44a	##364
.float		439a0ea7 , c3b5ebf6	##365
.float		453554ff , 458a47fa	##366
.float		457382ca , 451dd541	##367
.float		c452885a , c447f35e	##368
.float		431187d9 , 44d89ab9	##369
.float		c47f3f99 , 42890e33	##370
.float		c4f7d43a , 44908ba7	##371
.float		44298fd5 , c560c753	##372
.float		c3728608 , 44d9127c	##373
.float		c4d53d54 , 4353caf8	##374
.float		44d38100 , 4420af5a	##375
.float		c451e37b , 44d1f126	##376
.float		43059025 , 45c0a5c5	##377
.float		c3b6658c , 4314b9c6	##378
.float		c580cf61 , c203f193	##379
.float		c40d3eab , 43978b30	##380
.float		c4b13f88 , 4591c320	##381
.float		c520b540 , 4588477f	##382
.float		c5033d4a , 426f5da3	##383
.float		448e18a1 , c5617ba8	##384
.float		c4175919 , c5726ec7	##385
.float		441ee231 , 450c8dd5	##386
.float		44871bbb , 4508409e	##387
.float		c4cdeef7 , 454290db	##388
.float		45897415 , 452ca469	##389
.float		c43ba03e , 43c13afe	##390
.float		450705fe , 452b0819	##391
.float		44ebf052 , c3b792bf	##392
.float		c3872fca , 458b54e4	##393
.float		c4ed90ea , c538da94	##394
.float		c4d27afe , 43084a7e	##395
.float		c4c8254e , c459d4f5	##396
.float		c55fa409 , 4545e279	##397
.float		c573402a , c2e8a207	##398
.float		448c13f1 , c5a794a0	##399
.float		42febe0b , 45a6f546	##400
.float		458a96cd , 45814642	##401
.float		450c1960 , 44c2aead	##402
.float		4212d143 , 45539896	##403
.float		455c729a , c4f49a21	##404
.float		4369678f , 43311bbd	##405
.float		c566f6ba , c55c477f	##406
.float		43d03072 , c3c13112	##407
.float		c36bc40a , 4505d959	##408
.float		4323ed15 , 450f504c	##409
.float		4562c1ee , 44b2a516	##410
.float		44b295ae , 4485e0e7	##411
.float		c4f237ed , 4409c130	##412
.float		4503b662 , 44a58380	##413
.float		c4ec5e76 , 42b93a3e	##414
.float		44edba2a , 44449032	##415
.float		c48006c0 , 446d8365	##416
.float		4556699e , c4855903	##417
.float		c5570e41 , 44947caf	##418
.float		c48aa3d1 , 4447012d	##419
.float		c5c3389b , c449aa2b	##420
.float		449b305a , c4456715	##421
.float		458ffb35 , c53dee6e	##422
.float		43fa529e , c482f4b8	##423
.float		c43b7e09 , c4ce05b5	##424
.float		c5a9ff15 , c3e22c88	##425
.float		c4a7532a , 44ff540c	##426
.float		c4fd920b , c4a5fc4f	##427
.float		45138e10 , c4a0e33b	##428
.float		c44cf809 , c4ce04da	##429
.float		4362df85 , 4542d8c7	##430
.float		c3b14170 , 448006d1	##431
.float		44e20476 , 4482d37e	##432
.float		45026f04 , c5013a70	##433
.float		4577c328 , 44b6e317	##434
.float		43dda0fc , 44e4de56	##435
.float		43a50799 , c3e3c8a6	##436
.float		424a1b4e , c5053444	##437
.float		c503c4ed , 43a010d6	##438
.float		c5bfe789 , c4c30513	##439
.float		c511220e , 4475ec2e	##440
.float		45323f2f , 458ea7ce	##441
.float		c3a68f9c , c56aa360	##442
.float		c59c03ee , c50316a7	##443
.float		454dc7ce , c53b9392	##444
.float		c385cf4f , 4595746f	##445
.float		c385a594 , 457327b0	##446
.float		c4bfd67b , 453f5a39	##447
.float		c509deea , 457fcb6e	##448
.float		c58284df , 44a4b8ad	##449
.float		c4b2ed6f , 433607a7	##450
.float		c5be655b , c50a57da	##451
.float		459c7eaa , c48fdce1	##452
.float		c4a95cb2 , c4c45e4a	##453
.float		c53f8e18 , 449f5171	##454
.float		43644159 , 44ce0fe1	##455
.float		4595b7c9 , c54037b8	##456
.float		c5611b53 , 442e56c1	##457
.float		453df109 , c54b7fae	##458
.float		4545ba2b , 45408df9	##459
.float		c568d127 , 441530a5	##460
.float		c4831f13 , c52d3791	##461
.float		44f81040 , c49bc4ff	##462
.float		c572da12 , 457080f6	##463
.float		c4c87f7a , c3e4f970	##464
.float		44b814fe , 44e81e7d	##465
.float		44779496 , c55c6da1	##466
.float		c5294131 , c4f7bf58	##467
.float		452c6c8a , 4558010b	##468
.float		450877ca , 45833ee4	##469
.float		451f306e , c38d8043	##470
.float		44f0b52f , c5609115	##471
.float		455a52c1 , 44855b95	##472
.float		451d32e1 , c57b3958	##473
.float		45565b81 , c524c77f	##474
.float		44a2c1b2 , c49bd2dd	##475
.float		45408b64 , c4e6c9c2	##476
.float		c6020ee4 , 456e196f	##477
.float		c47ce2ac , 44701d0b	##478
.float		c5635f21 , 4502e647	##479
.float		c572e5f3 , c42f0e3f	##480
.float		44ab3530 , c562d0f5	##481
.float		c41c675b , c4a5174b	##482
.float		44566c3b , 4359eb25	##483
.float		c56430e2 , 450f1917	##484
.float		c54dfbe1 , c52b998e	##485
.float		c4a3f3f3 , c5ade7b6	##486
.float		454e048e , c4b9dc0a	##487
.float		45667c4e , 44277ee1	##488
.float		44861e89 , c4d13de3	##489
.float		45465b05 , c5b6be17	##490
.float		450a795a , c51b5f85	##491
.float		44a39db0 , 448bd7ec	##492
.float		4543cff9 , c58bdfe6	##493
.float		c49c3e18 , c468da22	##494
.float		c57e9b8e , c4e33919	##495
.float		c5169f95 , 4520ce3f	##496
.float		4579cc7a , 4502790a	##497
.float		45584337 , c5a132cb	##498
.float		452d1f3e , c4a67ba0	##499
.float		c4e91248 , 43d7d41a	##500
.float		454b3993 , 44a1657c	##501
.float		c540c07e , c4c781d2	##502
.float		c4c78bf8 , 455b1f7c	##503
.float		42808462 , c40230d1	##504
.float		442e1add , c58dbde7	##505
.float		c4a61bae , 4409a88c	##506
.float		44dca0f4 , c5072ed6	##507
.float		4532cecd , c3e0403a	##508
.float		44959753 , c561776d	##509
.float		c523b7d8 , 418b7d31	##510
.float		c5884208 , c54f84cc	##511
.float		43a17c30 , c55394a9	##512
.float		441e6238 , 420d1752	##513
.float		c575e2cb , 44fae01d	##514
.float		c5ad9f2b , c3c552b1	##515
.float		41a6d4a6 , c57df4d7	##516
.float		c557d0be , 433bfdd0	##517
.float		c492885b , 44fae752	##518
.float		44ce0632 , c447b47a	##519
.float		c4c2e6e9 , 43ce4b84	##520
.float		457309ad , c5171d5d	##521
.float		c3719fd2 , c516bd59	##522
.float		c4fca977 , c4de19af	##523
.float		43fa621c , c3fb815d	##524
.float		c5383436 , c4861e40	##525
.float		c50e0a19 , c2b931fe	##526
.float		45322cb3 , 45407ad8	##527
.float		448dffe1 , 45d76fa5	##528
.float		44dc9e30 , 44a799cb	##529
.float		455e9b15 , c571d0ea	##530
.float		c4d66f2c , c4b1799a	##531
.float		44c8abc0 , c5209392	##532
.float		45036b8f , c4bf956a	##533
.float		43f976a4 , c3f33037	##534
.float		457e6652 , 43f6423d	##535
.float		44cb5eb4 , 449260f6	##536
.float		43de382b , c4a27ae3	##537
.float		c417a025 , c331f3e2	##538
.float		454272b8 , c4960423	##539
.float		c4be5506 , c44ceed8	##540
.float		44fc3367 , c4ad0763	##541
.float		44d643b4 , 455fe8cd	##542
.float		c4480353 , 44ceebec	##543
.float		448dc077 , c57936f8	##544
.float		446af0f1 , 42b5c63f	##545
.float		c3d00937 , 4576e4bd	##546
.float		43f01df5 , c52140af	##547
.float		c55784c0 , 4589a6d1	##548
.float		c53baeab , 45042f9b	##549
.float		4411f157 , 45223c85	##550
.float		45dca972 , c42caf26	##551
.float		c50352a1 , 453ee41c	##552
.float		c5836cd2 , 45c539de	##553
.float		c578415b , 453365c2	##554
.float		43db6981 , c504113e	##555
.float		4591f746 , 45aa59b5	##556
.float		456ab4b4 , c406429b	##557
.float		4500771a , 45132397	##558
.float		4443be93 , c4db3e2e	##559
.float		4479f9cf , 44218f8d	##560
.float		c54f9b78 , 44368260	##561
.float		44bd6bb9 , c531b241	##562
.float		c42dbaee , c241436c	##563
.float		43f1de6a , c5506751	##564
.float		c45bc90a , c409b4cc	##565
.float		c51e6e47 , 439e7e71	##566
.float		c41e05aa , 4533cfee	##567
.float		451eea8e , 4548b5a3	##568
.float		44af9460 , 4540ff08	##569
.float		c4fef5b0 , c3adfdad	##570
.float		45035f19 , c48ebdc5	##571
.float		c5a3defd , c49aabbf	##572
.float		44a606bb , c528459a	##573
.float		c51e6bcc , 4547f1b9	##574
.float		452c237c , c57b1c0c	##575
.float		44c6bc62 , c542f110	##576
.float		c51332b1 , c51581ed	##577
.float		c3e8c2e2 , c4a68635	##578
.float		c408ab93 , 42f046dd	##579
.float		45b7d9dd , c3b44089	##580
.float		44efa0db , c593b91d	##581
.float		4507d85a , 44e72f77	##582
.float		442878dc , c4a1da4c	##583
.float		c553914d , 4470dd34	##584
.float		447269f4 , c4d9caa1	##585
.float		c409d1f0 , 44b4742f	##586
.float		452bbc2b , c41a8c1a	##587
.float		449c7b78 , 44c5cde1	##588
.float		457bc01e , 43d6e07e	##589
.float		c5457be1 , 45623af1	##590
.float		c46f4054 , 450c3ef4	##591
.float		44ec5ff9 , 454eb06d	##592
.float		c4ea8e34 , c41b23c6	##593
.float		c54ff95b , c4608469	##594
.float		456dccde , 44cd5a75	##595
.float		c4e20265 , 43d99db6	##596
.float		c59c0877 , 4337ed1c	##597
.float		c416cbcc , 457ae6a9	##598
.float		44c78c3a , 45a532a2	##599
.float		c42344ca , 455cbb7e	##600
.float		45a54d8a , c50c9dc9	##601
.float		45aef07b , 44fe7cf8	##602
.float		c347a570 , c5491229	##603
.float		44ecff27 , 4545e171	##604
.float		c50bb7a7 , c5a7a0bc	##605
.float		c384a9e4 , 4591af15	##606
.float		c4da1e61 , 43d480fd	##607
.float		c5658dd3 , c503fae1	##608
.float		450b2476 , 44d7e48c	##609
.float		c5a5e7c2 , 44db34ff	##610
.float		44f8514f , 453bae98	##611
.float		45753675 , 44691052	##612
.float		c4fdf852 , c5c1cf21	##613
.float		4577d13c , 450a4a11	##614
.float		c52ab323 , c4500a7a	##615
.float		c58986b5 , c5e7db0f	##616
.float		45a40078 , c542c382	##617
.float		c5367e87 , 44be5a22	##618
.float		c5478d9f , c48134ec	##619
.float		448b872e , 448f81ef	##620
.float		c4b09a50 , 45195846	##621
.float		458f23a9 , c5a94840	##622
.float		4519ce1a , 4473f411	##623
.float		c4959962 , 440ad319	##624
.float		44838504 , 45063449	##625
.float		c363ad40 , c51ee566	##626
.float		c3858c65 , 458d067c	##627
.float		446f088b , c5c0985f	##628
.float		c50b44b5 , c55ad349	##629
.float		c574b357 , 45db552a	##630
.float		44467830 , c4f7ebb7	##631
.float		c500ca78 , c4e083d9	##632
.float		44fef036 , c5959fba	##633
.float		4500b99a , 45704103	##634
.float		c4a0dc74 , 44a06faa	##635
.float		c592abc8 , c34f77bc	##636
.float		44c5bccf , 44e0e841	##637
.float		4318bcbc , c42c171f	##638
.float		c5241d68 , 42be9e8a	##639
.float		45b68d2e , 451e7f8e	##640
.float		4445d793 , c58ad707	##641
.float		c3992312 , c45d31d2	##642
.float		44080825 , 42bee7a4	##643
.float		456cabe8 , c38f4108	##644
.float		c407ca5f , 45918e0c	##645
.float		44eb095b , 45569074	##646
.float		c36e6225 , 44a39011	##647
.float		c35ed32d , 43586192	##648
.float		c51e5f41 , 4340f3a0	##649
.float		c5826f23 , 44995520	##650
.float		c50abfcf , c3a41cf4	##651
.float		4597600b , 450661f9	##652
.float		c54e35b3 , c4daf9c4	##653
.float		4452e903 , c55a5281	##654
.float		c4a97b32 , c4b36297	##655
.float		45808ee5 , 4542fc18	##656
.float		c4979b47 , c2b8c9d4	##657
.float		c2b8696b , c247450b	##658
.float		446e2385 , c527a229	##659
.float		437f46b8 , 456f0719	##660
.float		454c788a , 453ca389	##661
.float		c46522c1 , c5a5eeb3	##662
.float		c5100da0 , c21c9e60	##663
.float		c4824c86 , c53edef8	##664
.float		45308543 , 4567463a	##665
.float		c4463ba9 , 43d37e6b	##666
.float		c5351680 , c595fb9c	##667
.float		455ecb17 , c541970f	##668
.float		c46dbafe , c513f7cf	##669
.float		449cac79 , 4440c699	##670
.float		44929ffc , 44e2a9e6	##671
.float		440e275c , c5506201	##672
.float		c4931b2f , 44ee4688	##673
.float		44727d06 , 45bca35f	##674
.float		c38e6c77 , c5336d2a	##675
.float		4438cef9 , 4547c8cc	##676
.float		446c44df , c5f0921a	##677
.float		c40f3c92 , 44965ce7	##678
.float		c4c655e7 , 44f81bde	##679
.float		44d60aaa , 43a8907c	##680
.float		c46b6239 , c487454e	##681
.float		c49564c0 , c53923c2	##682
.float		c3a1c7b6 , c40ef379	##683
.float		4385b9dd , 45059576	##684
.float		446a858a , c506e460	##685
.float		4533a2ce , c47627bb	##686
.float		c26c36a6 , c39303a1	##687
.float		448a77d1 , 43db9ed8	##688
.float		445e8f8e , c5258095	##689
.float		c4bf83f3 , c5a518a6	##690
.float		4559d025 , c53c62d8	##691
.float		454d046f , 44975f9f	##692
.float		44a759d7 , 44d461b4	##693
.float		c2e94dc5 , 4470989a	##694
.float		430721d4 , c503c469	##695
.float		451c92cf , 4437316e	##696
.float		450aff21 , c5614060	##697
.float		c450e549 , c5056702	##698
.float		c4f51236 , c309104f	##699
.float		45191778 , 44fa646e	##700
.float		429274e5 , 4516d97a	##701
.float		c3dc24f1 , c58e4b67	##702
.float		c42995fe , 4480d95a	##703
.float		450dff5f , 453a6475	##704
.float		c4ad1bc0 , c504d684	##705
.float		45029ce7 , c53064f0	##706
.float		c31653db , 44c768b6	##707
.float		c594c185 , c4fdc6ec	##708
.float		c5470104 , 44cf0323	##709
.float		444cb31d , c5376e17	##710
.float		45246408 , 4451118b	##711
.float		c4124d0c , c58a39e4	##712
.float		451f74f1 , 451a88f1	##713
.float		c575c5ea , c530594b	##714
.float		c50c7f60 , c3cd78ea	##715
.float		c514e4b7 , 45030203	##716
.float		c562476d , 45ba8aa3	##717
.float		c49bb488 , 441a1a43	##718
.float		c580a6f9 , 448cdd1c	##719
.float		45566019 , c38c76a2	##720
.float		c53f30f9 , 44bb562f	##721
.float		44a212d6 , 45181493	##722
.float		c4496216 , 4535a948	##723
.float		c56ca916 , 447ea33a	##724
.float		c4df7e79 , 450e815b	##725
.float		441086c8 , c51f30e4	##726
.float		c45f8d2c , 44a815ce	##727
.float		c59819bb , c4ae1403	##728
.float		440a3f88 , c442fe61	##729
.float		435eb9ff , c4533d97	##730
.float		c4e90188 , c5011a00	##731
.float		45bdf3c1 , 450eeb29	##732
.float		c5716279 , c503c3b3	##733
.float		44801b7c , c5c90b58	##734
.float		c56f932f , c5353bce	##735
.float		c4a024ca , 45653beb	##736
.float		441389fe , 43d1404f	##737
.float		c547acfd , 455a3b3e	##738
.float		45345c1a , c4fa9318	##739
.float		451d353d , 43a814b6	##740
.float		44c9f085 , 428c6566	##741
.float		44185669 , 44e0b2ac	##742
.float		44ba8fe0 , c561bf17	##743
.float		c569300a , c32354dc	##744
.float		44d8e05a , 452a64c2	##745
.float		c5a79dcf , c53738f1	##746
.float		c3cfc42d , c5516176	##747
.float		c3cd5468 , 45485081	##748
.float		c5dcdc6e , 448f44da	##749
.float		c54c5336 , c38e7d85	##750
.float		c2960b50 , c4bf0ada	##751
.float		c4d57327 , c5622c6a	##752
.float		436d3692 , 450fc74d	##753
.float		c3c5e963 , c57f9c69	##754
.float		4528f76e , c56bde58	##755
.float		45c187fb , 4391c355	##756
.float		c5549b1b , c5921972	##757
.float		4386c0f8 , c4a8fc0a	##758
.float		c4c86294 , c4f8d521	##759
.float		456054c6 , 4511d3ae	##760
.float		c411e856 , c5951151	##761
.float		c3aad498 , c103215c	##762
.float		451079ad , c4868dce	##763
.float		43da8c42 , 43cb23a3	##764
.float		458dff13 , c53eb606	##765
.float		441071e2 , 45a385f5	##766
.float		44f0ac01 , 431c8575	##767
.float		c38502b0 , c56b62e2	##768
.float		c4697255 , 459d8107	##769
.float		456286f0 , 44c2013d	##770
.float		445ea8ee , c55efdba	##771
.float		44d6b64e , 4593d809	##772
.float		c531bece , c3af5f0f	##773
.float		c52559fe , 454ad31b	##774
.float		c498f41a , c3c48c75	##775
.float		4532e103 , 4521d633	##776
.float		4476a201 , c412dc29	##777
.float		c40c0e30 , 4501a49c	##778
.float		c56b49aa , 452acb4a	##779
.float		43523b1b , 45b58f71	##780
.float		4535d3e5 , c55b86af	##781
.float		4348f712 , c4c378ae	##782
.float		44810895 , c5749718	##783
.float		c5516b97 , 450acd58	##784
.float		c400d832 , c4fe5572	##785
.float		c54debb2 , 43c839f8	##786
.float		44a246a6 , c529eb3b	##787
.float		c5318387 , c428ae84	##788
.float		c55724c2 , c41160e4	##789
.float		448168fc , 44c5a11f	##790
.float		c5b138c6 , c56c6d66	##791
.float		c4cf087c , 4565b6b8	##792
.float		44782c72 , 455f6bc2	##793
.float		c5460266 , c4734c17	##794
.float		c4d9eda3 , 450c66e2	##795
.float		c484b60c , 43a4b8b3	##796
.float		c41727ab , 448a1d1d	##797
.float		c496d0ae , 454006c5	##798
.float		c5ed8fe1 , 45668257	##799
.float		45492fdf , c4efca40	##800
.float		448ce3ac , 441e6e53	##801
.float		c554174b , c58ca3f9	##802
.float		c432582f , 44247062	##803
.float		c41b071c , 4512a5ad	##804
.float		455d194a , c4ce2088	##805
.float		45313fe0 , 4364f6b5	##806
.float		c46ae781 , c320f1c4	##807
.float		c49cac66 , c4729055	##808
.float		c36b921f , c3463ce9	##809
.float		45697d8f , 44080ac4	##810
.float		45401249 , c50020b7	##811
.float		c40a8308 , 44bcbaec	##812
.float		4421f3e9 , c4224a3c	##813
.float		c291d4a6 , c45b9c2a	##814
.float		c4eb220c , c59203b3	##815
.float		4564bab0 , c3d159ce	##816
.float		c3818572 , c41f9f7d	##817
.float		44cbb6b4 , c5548478	##818
.float		c591a738 , 452d4428	##819
.float		c46a5e3e , 4589b2f8	##820
.float		c4a2dbd3 , 44bb255a	##821
.float		c481de09 , c47ea436	##822
.float		448e4a49 , 43fff52d	##823
.float		c598b053 , 455f30d4	##824
.float		c465b6ae , 458feaee	##825
.float		c3945771 , 44630a42	##826
.float		433499f7 , c430f871	##827
.float		c49377b2 , 456a95bc	##828
.float		452db5e2 , 44ed0561	##829
.float		c423bc0d , 43b0d3b0	##830
.float		45c00029 , c3ebd5de	##831
.float		453caade , c57cb4d0	##832
.float		43b4d7d8 , c547854d	##833
.float		c4ffdf4e , 456deda7	##834
.float		c3b7fe27 , c4d7d7cc	##835
.float		c4b01558 , 4407b6e3	##836
.float		c5539a2f , c420451d	##837
.float		441b051c , c41eb3b9	##838
.float		c4b44163 , c528935e	##839
.float		c4ffc126 , c52e44bc	##840
.float		c49a161a , c543b3f4	##841
.float		450cb02b , c28dc6a9	##842
.float		c49d0059 , 44dacf30	##843
.float		450a0e01 , 4409903f	##844
.float		c5d2119d , 452aa2fa	##845
.float		c4a6cd86 , 45517819	##846
.float		44841a24 , 45535221	##847
.float		c3fd5862 , c523504b	##848
.float		440069f0 , 439dc852	##849
.float		441410a7 , c4df530a	##850
.float		45072a68 , c53b1edf	##851
.float		459a9c32 , 447ff6cb	##852
.float		448150bc , c58a6ef3	##853
.float		442cc41e , 4339107a	##854
.float		44d67d7e , 4545b538	##855
.float		c52334bd , 448ae9d9	##856
.float		c5081383 , 42d388ff	##857
.float		458972be , c48c8ce1	##858
.float		4447c5e0 , c54ed21b	##859
.float		c52c98be , c460f91e	##860
.float		458c3683 , c3fc324a	##861
.float		44f5f19b , 45803276	##862
.float		c2bf86ff , 45518ddd	##863
.float		c42f02fb , 45372806	##864
.float		443a6988 , c453a15a	##865
.float		c4c64c00 , 424d0a41	##866
.float		453dabf6 , 4566231d	##867
.float		c52aee0e , 43879a57	##868
.float		450e83be , 44613357	##869
.float		45846b9a , 453805b9	##870
.float		44eaf817 , c3df567a	##871
.float		c4d64033 , 455f4e5d	##872
.float		45842985 , c4d4c4e7	##873
.float		c43d141a , 44934bc5	##874
.float		4596ead0 , 44d36b8f	##875
.float		44725301 , c549707f	##876
.float		c5822017 , 45014e59	##877
.float		c49f1fc7 , c50277c3	##878
.float		4477c518 , 450dc76d	##879
.float		c2a1c51c , c4a2ef46	##880
.float		c4f94024 , c510a607	##881
.float		c4f6088b , 4572a760	##882
.float		c4b44da0 , c5412c0f	##883
.float		c47e0210 , 43f096d8	##884
.float		c54a4171 , c50eb62e	##885
.float		4553c24c , 44d4b634	##886
.float		4464f66f , c4ad5597	##887
.float		4556bc4d , 424dce76	##888
.float		c50d918b , 44fb5ffe	##889
.float		c49cb158 , c2ccbfed	##890
.float		4522726e , c5afa72c	##891
.float		c4ff40d3 , c526908f	##892
.float		4581d02e , 44c9e397	##893
.float		42dcb268 , 44394bb9	##894
.float		c42a539c , c48fd08e	##895
.float		c4a66f09 , 44ae6f3f	##896
.float		c48577fd , 45165db8	##897
.float		c43a3680 , c5786746	##898
.float		4403c896 , c496bb38	##899
.float		c4f53634 , 445794ac	##900
.float		4399d080 , 4477d16f	##901
.float		c496ee83 , c53f4ab6	##902
.float		c41b56db , 4566ab79	##903
.float		c4dcf0c4 , 44a96593	##904
.float		c52f5777 , c52fa61f	##905
.float		c55ecc5f , 4530f410	##906
.float		4501b09f , c3138a69	##907
.float		c53de489 , c43bccfd	##908
.float		44743733 , c5139331	##909
.float		4511932f , c500878f	##910
.float		45968c65 , c3bef555	##911
.float		c5449307 , 429f0619	##912
.float		c58746f8 , c30e5df1	##913
.float		c3d97c0a , c540cfa7	##914
.float		c4915516 , c539e804	##915
.float		44b107cd , 4503c390	##916
.float		433e0287 , 454c5034	##917
.float		44c83049 , c44660ab	##918
.float		c5655058 , c5814f2c	##919
.float		c494fbf5 , c484cfc5	##920
.float		c5a2b98c , c3f7fe93	##921
.float		c55eea65 , 449e7bb1	##922
.float		44e582c5 , c52e6cd3	##923
.float		44d89346 , 4352162d	##924
.float		c5025cd1 , c43af4d7	##925
.float		4485dbc4 , 440bef23	##926
.float		c5ee65fe , c48eee4b	##927
.float		c4947ebb , c2d81f21	##928
.float		c50e3e48 , 453c2ebc	##929
.float		44e524d0 , c4827c0e	##930
.float		41802834 , 4587dcdc	##931
.float		c40c3210 , c4d0ba38	##932
.float		44ce325e , c580c07c	##933
.float		458be95d , c45e5e81	##934
.float		c55fe938 , 456275a5	##935
.float		c54a23ea , c4f9301b	##936
.float		448cd0cf , c4257d84	##937
.float		c3d0265e , 452e8bd9	##938
.float		c4396fca , c49d116c	##939
.float		c553a615 , c556e1cd	##940
.float		c41590ff , c3d7cfad	##941
.float		456a8eb7 , 453201d5	##942
.float		456af86c , c3bf61e4	##943
.float		c5a03448 , c3fe5e47	##944
.float		448eb1a8 , c5183d6b	##945
.float		c586878e , c47d1d79	##946
.float		c5bf9406 , c4c85a8a	##947
.float		c5036929 , 44a79486	##948
.float		451670e5 , c553a292	##949
.float		440c0401 , 44b18fa3	##950
.float		c5050008 , 44b67e33	##951
.float		c1dca8eb , 458e0f70	##952
.float		c50ea0e1 , c51c424f	##953
.float		447d53ae , c54731f1	##954
.float		4589cff6 , c5102fee	##955
.float		c554b155 , 440b81cd	##956
.float		c42d4955 , c5152725	##957
.float		c4e35c51 , 4566e8e4	##958
.float		45810acb , c59ece0f	##959
.float		44bc5e25 , 44ff2f2b	##960
.float		c4fc6b0c , 44531f97	##961
.float		459a95a9 , c49d288d	##962
.float		440c45b9 , c4912c45	##963
.float		c3daf5ea , c58a7478	##964
.float		451656b3 , c48520c1	##965
.float		42698515 , c495e6fc	##966
.float		45772ba3 , 4516950e	##967
.float		444ac576 , 458a35b0	##968
.float		430d5bb6 , c379dc27	##969
.float		439fd3c9 , c5c9bd76	##970
.float		44d5828a , c573b7fa	##971
.float		c5447ff4 , 459f1afd	##972
.float		c470249d , 45392bb3	##973
.float		44ba3b59 , 45773673	##974
.float		c4a5af64 , c4c87b05	##975
.float		4551037a , c4a81ffc	##976
.float		4345a455 , c408b5cd	##977
.float		458f4411 , 45807d24	##978
.float		45a3510a , c493fded	##979
.float		c3a36adf , c4f0e9f5	##980
.float		43f31a81 , c4f52980	##981
.float		c53527e6 , c2d350df	##982
.float		c4d062f6 , c3b836e0	##983
.float		c5a3ce66 , 449e25f5	##984
.float		4430e20e , 4548a612	##985
.float		c52b6b29 , c57d993c	##986
.float		440b6e1a , 4466bce7	##987
.float		c28fead2 , c5c31bdf	##988
.float		45020b12 , c3c074cf	##989
.float		434ca273 , c4a27766	##990
.float		c5295e9a , c471ca0e	##991
.float		44435a96 , 42af2eb8	##992
.float		c4f78b16 , 4545429b	##993
.float		448d02fd , c53b3bb1	##994
.float		4540ca5d , 43a77a38	##995
.float		45025660 , 44a3a5fb	##996
.float		c51016aa , c529bacf	##997
.float		c4d5578b , 455afd2f	##998
.float		c5973837 , 450c649e	##999
.float		4429bba7 , c518a67c	##1000
.float		c51cf326 , c482205c	##1001
.float		4516fe5d , c4112671	##1002
.float		3d398a83 , c56206ed	##1003
.float		c4938dc1 , 4580d408	##1004
.float		44f593b6 , 40265366	##1005
.float		45c86c24 , c5379d00	##1006
.float		4556f295 , 444a2107	##1007
.float		454c7aaf , c53e0b92	##1008
.float		c517e498 , 4513bc7d	##1009
.float		c4f78cbf , c494331c	##1010
.float		451df32c , 440e4d19	##1011
.float		4578ab59 , c4063b25	##1012
.float		420189b5 , c3673220	##1013
.float		44b7dd4f , 44fa897a	##1014
.float		44e6911b , c34698a5	##1015
.float		415bd539 , c483eb60	##1016
.float		c4a4147a , c55f1018	##1017
.float		441b2653 , c3125e2f	##1018
.float		44ec212d , c524df68	##1019
.float		43f3ba09 , c48f53b2	##1020
.float		c4be4e6d , 44166c73	##1021
.float		452aba92 , c50a28b2	##1022
.float		c3430285 , 446f4307	##1023
.float		450daf1b , c54fdaf9	##1024
