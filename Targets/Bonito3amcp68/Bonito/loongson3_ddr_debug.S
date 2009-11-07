#if 0
	TTYDBG("Write 0x90000f00000000 s\r\n")
    dli     t0 , 0x900000003ff02000
  dli a0, 0x0000100000000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02040
  dli a0, 0xfffff00000000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02080
  dli a0, 0x00001000000000f0
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02008
  dli a0, 0x0000200000000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02048
  dli a0, 0xffffe00000000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02088
  dli a0, 0x00002000000000f0
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02010
  dli a0, 0x0000000010000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02050
  dli a0, 0xfffffffffc000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02090
  dli a0, 0x00000e0000000087
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02018
  dli a0, 0x0000000010000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02058
  dli a0, 0xfffffffff8000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02098
  dli a0, 0x00000e0010000087
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02020
  dli a0, 0x000000001a000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02060
  dli a0, 0xffffffffff000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff020a0
  dli a0, 0x00000efdfc000087
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02028
  dli a0, 0x00000e0000000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02068
  dli a0, 0xffffff0000000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff020a8
  dli a0, 0x00000e0000000087
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02030
  dli a0, 0x00000c0000000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02070
  dli a0, 0xfffffc0000000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff020b0
  dli a0, 0x00000c00000000f0
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02038
  dli a0, 0x000000001b000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff02078
  dli a0, 0xffffffffff000000
  sd a0, 0x0(t0)
    dli     t0 , 0x900000003ff020b8
  dli a0, 0x00000efdfb000087
  sd a0, 0x0(t0)

//     dli t0,  0x9000000000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x9000020000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x9000040000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x9000060000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x9000080000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x90000a0000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x90000c0000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x90000d0000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x90000e0000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x90000f0000000000
// ld    a0, 0x0(t0)

//     dli t0,  0x9800000000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x9800020000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x9800040000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x9800060000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x9800080000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x98000a0000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x98000c0000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x98000d0000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x98000e0000000000
// ld    a0, 0x0(t0)
//     dli t0,  0x98000f0000000000
// ld    a0, 0x0(t0)



    dli     t0,  0x98000f0000000000
1:
    ld      a0,  0x0(t0)
    bal     hexserial
    nop
    li      a0,  0x100000
    dadd    t0, t0, a0
	TTYDBG("\r\n")
    b       1b
    nop
    
    

#endif




#if 0
	TTYDBG("Write all 0s\r\n")
//Write all 00000000
        li      t0, 0xa0000000
        li      a0, 0x0
        li      t2, 0xa0000020
1:
        sw      t0, 0x0(t0)
        addi    t0, 4
        bne     t0, t2, 1b
        nop
        
//Read
        li      t0, 0xa0000000
        li      a0, 0xffffffff
        li      t2, 0xa0000020
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0x80000000
        li      a0, 0xffffffff
        li      t2, 0x80000020
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop
#endif

#if 0 /* write and read whd */
	TTYDBG("Write and write\r\n")
    li  t0, 0xa0010000
    //li  t1, 0x12345678
    //li  t1, 0x40806000
    li  t1, 0x0
    li  t2, 0x0
    lw  a0, 0x0(t0)
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
1:
    //addi t1, t1, 0x1
    addi t2, t2, 0x1
    sw  t1, 0x0(t0)
    lw  a0, 0x0(t0)
    beq t1, a0, 1b
    nop
	    bal	hexserial
        nop
	    TTYDBG("    ")
        move    a0, t2
	    bal	hexserial
        nop
	    TTYDBG("\r\n")

        b 1b
        nop
#endif





#if 0
uncache_test:
//UNCACHE WRITE whd
	TTYDBG("Write all address\r\n")
//Write all 00000000
22:
        li      t0, 0xa0001400
        li      a0, 0x0
        li      t2, 0xa0001420
1:
        andi    a0, t0, 0xff
        sll     a1, a0, 8
        or      a1, a0, a1
        sll     a0, a1, 16
        or      a0, a0, a1
        sw      a0, 0x0(t0)
        addi    t0, 4
        bne     t0, t2, 1b
        nop
        //b       22b
        nop
        

#if 1 /* write 420 */
//Write all 00000000
        li      t0, 0xa0001420
        li      a0, 0x0
        li      t2, 0xa0001440
1:
        andi    a0, t0, 0xff
        sll     a1, a0, 8
        or      a1, a0, a1
        sll     a0, a1, 16
        or      a0, a0, a1
        sw      a0, 0x0(t0)
        addi    t0, 4
        bne     t0, t2, 1b
        nop
        
#endif


//Read
	    TTYDBG("    ")
        li      t0, 0xa0001400
        li      a0, 0xffffffff
        li      t2, 0xa0001420
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        andi    a0, t0, 0x4
        bnez    a0, 11f
	    TTYDBG("    \r\n")
11:
	    TTYDBG("    ")
        bne     t0, t2, 1b
        nop
	    TTYDBG("Next cached\r\n")


//Read
	    TTYDBG("    ")
        li      t0, 0x80001400
        li      a0, 0xffffffff
        li      t2, 0x80001420
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        andi    a0, t0, 0x4
        bnez    a0, 11f
	    TTYDBG("\r\n")
11:
	    TTYDBG("    ")
        bne     t0, t2, 1b
        nop
	    TTYDBG("Next 420\r\n")

/*
        li      t0, 0x80001400
        cache   23, 0x0(t0)
        nop
        sync
        nop
        */

#if 1 /* write 440 */
//Read
	    TTYDBG("    ")
        li      t0, 0xa0001420
        li      a0, 0xffffffff
        li      t2, 0xa0001440
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        andi    a0, t0, 0x4
        bnez    a0, 11f
	    TTYDBG("\r\n")
11:
	    TTYDBG("    ")
        bne     t0, t2, 1b
        nop

//Write all 00000000
        li      t0, 0xa0001440
        li      a0, 0x0
        li      t2, 0xa0001460
1:
        andi    a0, t0, 0xff
        sll     a1, a0, 8
        or      a1, a0, a1
        sll     a0, a1, 16
        or      a0, a0, a1
        sw      a0, 0x0(t0)
        addi    t0, 4
        bne     t0, t2, 1b
        nop
        


	    TTYDBG("Next 440\r\n")
//Read
	    TTYDBG("    ")
        li      t0, 0xa0001440
        li      a0, 0xffffffff
        li      t2, 0xa0001460
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        andi    a0, t0, 0x4
        bnez    a0, 11f
	    TTYDBG("\r\n")
11:
	    TTYDBG("    ")
        bne     t0, t2, 1b
        nop
#endif




2:  

        b       uncache_test
        nop
#endif

#if 0 //LOOP whd
33:
        li      t0, 0x80000400
        li      a0, 0x0
        li      t2, 0x80000420
1:
#if 1
        li      a0, 0xffffffff
        sw      a0, 0x0(t0)
        li      a0, 0xffffffff
        sw      a0, 0x4(t0)
        li      a0, 0x00000000
        sw      a0, 0x8(t0)
        li      a0, 0x00000000
        sw      a0, 0xc(t0)
        li      a0, 0xffffffff
        sw      a0, 0x10(t0)
        li      a0, 0xffffffff
        sw      a0, 0x14(t0)
        //li      a0, 0x00000000
        sw      a0, 0x18(t0)
        //li      a0, 0x00000000
        sw      a0, 0x1c(t0)
#else
        li      a0, 0x03508023
        sw      a0, 0x0(t0)
        li      a0, 0x3c01ffff
        sw      a0, 0x4(t0)
        li      a0, 0x02018024
        sw      a0, 0x8(t0)
        li      a0, 0x3c080040
        sw      a0, 0xc(t0)
        li      a0, 0x03508023
        sw      a0, 0x10(t0)
        li      a0, 0x3c01ffff
        sw      a0, 0x14(t0)
        li      a0, 0x02018024
        sw      a0, 0x18(t0)
        li      a0, 0x3c080040
        sw      a0, 0x1c(t0)
/*
        li      a0, 0x0
        sw      a0, 0x0(t0)
        sw      a0, 0x4(t0)
        li      a0, 0xffffffff
        sw      a0, 0x8(t0)
        sw      a0, 0xc(t0)
        li      a0, 0x0
        sw      a0, 0x10(t0)
        sw      a0, 0x14(t0)
        li      a0, 0xffffffff
        sw      a0, 0x18(t0)
        sw      a0, 0x1c(t0)
        */
#endif
        //addi    t0, 4
        //bne     t0, t2, 1b
        //nop
        
        li      t0, 0x80000400
        cache   23, 0x0(t0) //HitWBInvalidate_S

#if 1
        b       33b
        nop
#else
        sync
        nop

        li      t0, 0x80000400
        li      a1, 0x03508023
        lw      a0, 0x0(t0)
        bne     a0, a1, 1f
        nop
        li      a1, 0x3c01ffff
        lw      a0, 0x4(t0)
        bne     a0, a1, 1f
        nop
        li      a1, 0x02018024
        lw      a0, 0x8(t0)
        bne     a0, a1, 1f
        nop
        li      a1, 0x3c080040
        lw      a0, 0xc(t0)
        bne     a0, a1, 1f
        nop
        b       2f
        nop
1:
        bal     hexserial
        nop
2:
	TTYDBG(".")
        b       33b
        nop
#endif

#endif

#if 0
//CACHE WRITE whd
	TTYDBG("Write all address\r\n")
//Write all 00000000
        li      t0, 0x80000400
        li      a0, 0x0
        li      t2, 0x80000420
1:
        andi    a0, t0, 0xff
        sll     a1, a0, 8
        or      a1, a0, a1
        sll     a0, a1, 16
        or      a0, a0, a1
        sw      a0, 0x0(t0)
        addi    t0, 4
        bne     t0, t2, 1b
        nop
        
        li      t0, 0x80000400
        cache   23, 0x0(t0) //HitWBInvalidate_S

#if 1 /* write 420 */
//Write all 00000000
        li      t0, 0x80000420
        li      a0, 0x0
        li      t2, 0x80000440
1:
        andi    a0, t0, 0xff
        sll     a1, a0, 8
        or      a1, a0, a1
        sll     a0, a1, 16
        or      a0, a0, a1
        sw      a0, 0x0(t0)
        addi    t0, 4
        bne     t0, t2, 1b
        nop
        
        li      t0, 0x80000420
        cache   23, 0x0(t0) //HitWBInvalidate_S

#endif


//Read
	    TTYDBG("    ")
        li      t0, 0xa0000400
        li      a0, 0xffffffff
        li      t2, 0xa0000420
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        andi    a0, t0, 0x4
        bnez    a0, 11f
	    TTYDBG("    \r\n")
11:
	    TTYDBG("    ")
        bne     t0, t2, 1b
        nop
	    TTYDBG("Next cached\r\n")


//Read
	    TTYDBG("    ")
        li      t0, 0x80000400
        li      a0, 0xffffffff
        li      t2, 0x80000420
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        andi    a0, t0, 0x4
        bnez    a0, 11f
	    TTYDBG("\r\n")
11:
	    TTYDBG("    ")
        bne     t0, t2, 1b
        nop
	    TTYDBG("Next 420\r\n")

//Read
	    TTYDBG("    ")
        li      t0, 0x80000420
        li      a0, 0xffffffff
        li      t2, 0x80000440
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        andi    a0, t0, 0x4
        bnez    a0, 11f
	    TTYDBG("\r\n")
11:
	    TTYDBG("    ")
        bne     t0, t2, 1b
        nop

#if 1 /* write 440 */
//Write all 00000000
        li      t0, 0x80000440
        li      a0, 0x0
        li      t2, 0x80000460
1:
        andi    a0, t0, 0xff
        sll     a1, a0, 8
        or      a1, a0, a1
        sll     a0, a1, 16
        or      a0, a0, a1
        sw      a0, 0x0(t0)
        addi    t0, 4
        bne     t0, t2, 1b
        nop
        
        li      t0, 0x80000440
        cache   23, 0x0(t0) //HitWBInvalidate_S
#endif


	    TTYDBG("Next 440\r\n")
//Read
	    TTYDBG("    ")
        li      t0, 0x80000440
        li      a0, 0xffffffff
        li      t2, 0x80000460
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        andi    a0, t0, 0x4
        bnez    a0, 11f
	    TTYDBG("\r\n")
11:
	    TTYDBG("    ")
        bne     t0, t2, 1b
        nop




2:  

//#define ENDLESS_READ
#ifdef ENDLESS_READ
        li      t0, 0x80000400
        cache   23, 0x0(t0) //HitWBInvalidate_S
        li      t0, 0x80000420
        cache   23, 0x0(t0) //HitWBInvalidate_S
        li      t0, 0x80000440
        cache   23, 0x0(t0) //HitWBInvalidate_S

    //li      t1, 0x3fffffff #360Mhz
    li      t1, 0x1fffffff #750Mhz
    //li      t1, 0x1ffffff #24Mhz
1:
    bnez    t1, 1b
    addi    t1, t1, -1
    

//Read
	    TTYDBG("\r\n\n\nENDLESS READ\r\n")
	    TTYDBG("    ")
        li      t0, 0x80000400
        li      a0, 0xffffffff
        li      t2, 0x80000420
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        andi    a0, t0, 0x4
        bnez    a0, 11f
	    TTYDBG("\r\n")
11:
	    TTYDBG("    ")
        bne     t0, t2, 1b
        nop
	    TTYDBG("Next 420\r\n")

//Read
	    TTYDBG("    ")
        li      t0, 0x80000420
        li      a0, 0xffffffff
        li      t2, 0x80000440
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        andi    a0, t0, 0x4
        bnez    a0, 11f
	    TTYDBG("\r\n")
11:
	    TTYDBG("    ")
        bne     t0, t2, 1b
        nop
	    TTYDBG("Next 440\r\n")

//Read
	    TTYDBG("    ")
        li      t0, 0x80000440
        li      a0, 0xffffffff
        li      t2, 0x80000460
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        andi    a0, t0, 0x4
        bnez    a0, 11f
	    TTYDBG("\r\n")
11:
	    TTYDBG("    ")
        bne     t0, t2, 1b
        nop

#endif

        b   2b
        nop
#endif


//memory pattern test whd
#if 0
	TTYDBG("Write all 0s\r\n")
//Write all 00000000
        li      t0, 0xa0000000
        li      a0, 0x0
        li      t2, 0xa0000020
1:
        sw      t0, 0x0(t0)
        addi    t0, 0x4
        bne     t0, t2, 1b
        nop
        
//Read
        li      t0, 0xa0000000
        li      a0, 0xffffffff
        li      t2, 0xa0000020
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0xa0000000
        li      a0, 0xffffffff
        li      t2, 0xa0000020
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

	TTYDBG("Write a FFFFFF to 0x0\r\n")
//Write a FFFFFFF to 0x0
        li      t0, 0xa0000000
        li      a0, 0xffffffff
        sw      a0, 0x0(t0)
//Read
        li      t0, 0xa0000000
        li      a0, 0xffffffff
        li      t2, 0xa0000020
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0x80000000
        li      a0, 0xffffffff
        li      t2, 0x80000020
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

        
	TTYDBG("Write all FFFFFFFs\r\n")
//Write all FFFFFFF
        li      t0, 0xa0000100
        li      a0, 0xffffffff
        li      t2, 0xa0000120
1:
        sw      t0, 0x0(t0)
        addi    t0, 0x4
        bne     t0, t2, 1b
        nop
        
//Read
        li      t0, 0xa0000100
        li      a0, 0xffffffff
        li      t2, 0xa0000120
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0xa0000100
        li      a0, 0xffffffff
        li      t2, 0xa0000120
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

	TTYDBG("Write a 0 to 0x4\r\n")
//Write a FFFFFFF to 0x0
        li      t0, 0xa0000100
        li      a0, 0x0
        sw      a0, 0x4(t0)
//Read
        li      t0, 0xa0000100
        li      a0, 0xffffffff
        li      t2, 0xa0000120
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0x80000100
        li      a0, 0xffffffff
        li      t2, 0x80000120
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop
        

	TTYDBG("\r\nWrite all 1s\r\n")
//Write all 00000000
        li      t0, 0xa0000200
        li      a0, 0x11111111
        li      t2, 0xa0000220
1:
        sw      t0, 0x0(t0)
        addi    t0, 0x4
        bne     t0, t2, 1b
        nop
        
//Read
        li      t0, 0xa0000200
        li      a0, 0xffffffff
        li      t2, 0xa0000220
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0xa0000200
        li      a0, 0xffffffff
        li      t2, 0xa0000220
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

	TTYDBG("Write a 22222222 to 0x8\r\n")
//Write a FFFFFFF to 0x0
        li      t0, 0xa0000200
        li      a0, 0x22222222
        sw      a0, 0x8(t0)
//Read
        li      t0, 0xa0000200
        li      a0, 0xffffffff
        li      t2, 0xa0000220
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0x80000200
        li      a0, 0xffffffff
        li      t2, 0x80000220
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

        

	TTYDBG("Write all 4s\r\n")
//Write all 00000000
        li      t0, 0xa0000300
        li      a0, 0x44444444
        li      t2, 0xa0000320
1:
        sw      t0, 0x0(t0)
        addi    t0, 0x4
        bne     t0, t2, 1b
        nop
        
//Read
        li      t0, 0xa0000300
        li      a0, 0xffffffff
        li      t2, 0xa0000320
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

	TTYDBG("Write a 33333333 to 0xC\r\n")
//Write a FFFFFFF to 0x0
        li      t0, 0xa0000300
        li      a0, 0x33333333
        sw      a0, 0xc(t0)
//Read
        li      t0, 0xa0000300
        li      a0, 0xffffffff
        li      t2, 0xa0000320
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0x80000300
        li      a0, 0xffffffff
        li      t2, 0x80000320
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

        


	TTYDBG("Write all 8s\r\n")
//Write all 00000000
        li      t0, 0xa0000400
        li      a0, 0x88888888
        li      t2, 0xa0000420
1:
        sw      t0, 0x0(t0)
        addi    t0, 0x4
        bne     t0, t2, 1b
        nop
        
//Read
        li      t0, 0xa0000400
        li      a0, 0xffffffff
        li      t2, 0xa0000420
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

	TTYDBG("Write a 77777777 to 0x10\r\n")
//Write a FFFFFFF to 0x0
        li      t0, 0xa0000400
        li      a0, 0x77777777
        sw      a0, 0x10(t0)
//Read
        li      t0, 0xa0000400
        li      a0, 0xffffffff
        li      t2, 0xa0000420
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0x80000400
        li      a0, 0xffffffff
        li      t2, 0x80000420
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

        


	TTYDBG("Write all 6666666s\r\n")
//Write all 00000000
        li      t0, 0xa0000600
        li      a0, 0x66666666
        li      t2, 0xa0000620
1:
        sw      t0, 0x0(t0)
        addi    t0, 0x4
        bne     t0, t2, 1b
        nop
        
//Read
        li      t0, 0xa0000600
        li      a0, 0xffffffff
        li      t2, 0xa0000620
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

	TTYDBG("Write a aaaaaaaa to 0x14\r\n")
//Write a FFFFFFF to 0x0
        li      t0, 0xa0000600
        li      a0, 0xaaaaaaaa
        sw      a0, 0x14(t0)
//Read
        li      t0, 0xa0000600
        li      a0, 0xffffffff
        li      t2, 0xa0000620
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0x80000600
        li      a0, 0xffffffff
        li      t2, 0x80000620
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
        
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop



	TTYDBG("Write all 9999999s\r\n")
//Write all 00000000
        li      t0, 0xa0000700
        li      a0, 0x99999999
        li      t2, 0xa0000720
1:
        sw      t0, 0x0(t0)
        addi    t0, 0x4
        bne     t0, t2, 1b
        nop
        
//Read
        li      t0, 0xa0000700
        li      a0, 0xffffffff
        li      t2, 0xa0000720
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

	TTYDBG("Write a 66666666 to 0x18\r\n")
//Write a FFFFFFF to 0x0
        li      t0, 0xa0000700
        li      a0, 0x66666666
        sw      a0, 0x18(t0)
//Read
        li      t0, 0xa0000700
        li      a0, 0xffffffff
        li      t2, 0xa0000720
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0x80000700
        li      a0, 0xffffffff
        li      t2, 0x80000720
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

        


	TTYDBG("Write all ccccccs\r\n")
//Write all 00000000
        li      t0, 0xa0000800
        li      a0, 0xcccccccc
        li      t2, 0xa0000820
1:
        sw      t0, 0x0(t0)
        addi    t0, 0x4
        bne     t0, t2, 1b
        nop
        
//Read
        li      t0, 0xa0000800
        li      a0, 0xffffffff
        li      t2, 0xa0000820
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

	TTYDBG("Write a 33333333 to 0x1c\r\n")
//Write a FFFFFFF to 0x0
        li      t0, 0xa0000800
        li      a0, 0x33333333
        sw      a0, 0x1c(t0)
//Read
        li      t0, 0xa0000800
        li      a0, 0xffffffff
        li      t2, 0xa0000820
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

//Read
        li      t0, 0x80000800
        li      a0, 0xffffffff
        li      t2, 0x80000820
1:
        lw      a0, 0x0(t0)
        addi    t0, 0x4
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        bne     t0, t2, 1b
        nop

        


#endif

#if 0 /* read ddr registers */
        dli     t2, 0x900000000ff00000
        dli     t0, 0x900000000fff0000
1:
        ld      a0, 0x0(t2)
	    bal	hexserial
        nop
	    TTYDBG("\r\n")
        daddi   t2, 0x8

        bne     t2, t0, 1b
        nop
#endif

#if 1 //all space scan whd
#define SPACE_1G_AT_MC1
#ifdef 0
  //OPEN WINDOWS 256-512M @ 0x20000000
        dli     t2, 0x900000003ff00000
        dli     t0, 0x20000000
        sd      t0, 0x10(t2)
        dli     t0, 0xfffffffff0000000
        sd      t0, 0x50(t2)
        dli     t0, 0x100000f1
        sd      t0, 0x90(t2)
  //OPEN WINDOWS 512M-1G  @ 0x40000000
        dli     t2, 0x900000003ff00000
        dli     t0, 0x40000000
        sd      t0, 0x18(t2)
        dli     t0, 0xffffffffe0000000
        sd      t0, 0x58(t2)
        dli     t0, 0x200000f1
        sd      t0, 0x98(t2)
#endif

#if 0 /* print routing table */
	    TTYDBG("Windows config table\r\n")
        dli     t2, 0x900000003ff00000
        dli     t0, 0x900000003ff00100
1:
        ld      t1, 0x0(t2)
        dsrl    a0, t1, 32
        bal     hexserial
        nop
        move    a0, t1
        bal     hexserial
        nop
	    TTYDBG("\r\n")
        daddi   t2, t2, 8
        bne     t2, t0, 1b
        nop

	    TTYDBG("Windows config table\r\n")
        dli     t2, 0x900000003ff02000
        dli     t0, 0x900000003ff02100
1:
        ld      t1, 0x0(t2)
        dsrl    a0, t1, 32
        bal     hexserial
        nop
        move    a0, t1
        bal     hexserial
        nop
	    TTYDBG("\r\n")
        daddi   t2, t2, 8
        bne     t2, t0, 1b
        nop
#endif


	    TTYDBG("Space init begin\r\n")
#if 1
        dli     t2, 0x9000000000000000
        dli     t0, 0x9000000010000000
1:
        sd      t2, 0x0(t2)
        daddi   t2, 0x8
        bne     t2, t0, 1b
        nop
	    TTYDBG("Space init 0    - 256M Done\r\n")
#endif

#if 1
//
        dli     t2, 0x9000000020000000
        dli     t0, 0x9000000030000000
1:
        sd      t2, 0x0(t2)
        daddi   t2, 0x8
        bne     t2, t0, 1b
        nop
	    TTYDBG("Space init 256  - 512M Done\r\n")
        dli     t2, 0x9000000040000000
        dli     t0, 0x9000000060000000
1:
        sd      t2, 0x0(t2)
        daddi   t2, 0x8
        bne     t2, t0, 1b
        nop
	    TTYDBG("Space init 512M - 1G   Done\r\n")
        dli     t2, 0x9000000060000000
        dli     t0, 0x90000000a0000000
1:
        sd      t2, 0x0(t2)
        daddi   t2, 0x8
        bne     t2, t0, 1b
        nop
	    TTYDBG("Space init 1G   - 2G   Done\r\n")
#endif
	    TTYDBG("Space scan begin\r\n")
        dli     t2, 0x9000000000000000
        dli     t0, 0x9000000010000000
1:
        ld      a1, 0x0(t2)
        bne     a1, t2, scan_error
        nop
        daddi   t2, 0x8
        bne     t2, t0, 1b
        nop
	    TTYDBG("Space Done 0    - 256M Done\r\n")
        dli     t2, 0x9000000020000000
        dli     t0, 0x9000000030000000
1:
        ld      a1, 0x0(t2)
        bne     a1, t2, scan_error
        nop
        daddi   t2, 0x8
        bne     t2, t0, 1b
        nop
	    TTYDBG("Space Done 256  - 512M Done\r\n")
        dli     t2, 0x9000000040000000
        dli     t0, 0x9000000060000000
1:
        ld      a1, 0x0(t2)
        bne     a1, t2, scan_error
        nop
        daddi   t2, 0x8
        bne     t2, t0, 1b
        nop
	    TTYDBG("Space Done 512M - 1G   Done\r\n")
        dli     t2, 0x9000000060000000
        dli     t0, 0x90000000a0000000
1:
        ld      a1, 0x0(t2)
        bne     a1, t2, scan_error
        nop
        daddi   t2, 0x8
        bne     t2, t0, 1b
        nop
	    TTYDBG("Space Done 1G   - 2G   Done\r\n")
        b       2f
        nop
scan_error:
        
        dsrl    a0, a1, 32
        bal     hexserial
        nop
        move    a0, a1
        bal     hexserial
        nop
	    TTYDBG("   Correct Vaule :   ")

        dsrl    a0, t2, 32
        bal     hexserial
        nop
        move    a0, t2
        bal     hexserial
        nop

1:      
        b       1b
        nop


2:


#endif

//memory test cuckoo
#if 0
        li     t0,0xa0000000
        //li     t1,0xa55aa55a
        li     t1,0xffffffff
        li     t2,0xa0100000
write:
        sw     t0,0x0(t0)
        addi   t0,t0,0x4

        //li     t0,0xa0000100
        //not    t1
        //sd     t1,0x0(t0)
        bne	t0, t2, write
        nop

	TTYDBG("init memory done\r\n")
	//TTYDBG("init memory done\r\n")


1:
        li     t0,0xa0000000
        li     t2,0xa0010000
read:

        lw     t1,0x0(t0)

        //bne	t1, t0, display
   	//nop

        addi   t0,t0,0x4

        //lw      a0,0x0(t0)
        move   a0,t1
	bal	hexserial
        nop
	TTYDBG("\r\n")

        lw     t1,0x0(t0)
        addi   t0,t0,0x4

        //lw      a0,0x0(t0)
        move   a0,t1
	bal	hexserial
        nop
	TTYDBG("\r\n")

        lw     t1,0x0(t0)
        addi   t0,t0,0x4

        //lw      a0,0x0(t0)
        move   a0,t1
	bal	hexserial
        nop
	TTYDBG("\r\n")

        lw     t1,0x0(t0)
        addi   t0,t0,0x4

        //lw      a0,0x0(t0)
        move   a0,t1
	bal	hexserial
        nop
	TTYDBG("\r\n")

        lw     t1,0x0(t0)
        addi   t0,t0,0x4

        //lw      a0,0x0(t0)
        move   a0,t1
	bal	hexserial
        nop
	TTYDBG("\r\n")

        lw     t1,0x0(t0)
        addi   t0,t0,0x4

        //lw      a0,0x0(t0)
        move   a0,t1
	bal	hexserial
        nop
	TTYDBG("\r\n")

        lw     t1,0x0(t0)
        addi   t0,t0,0x4

        //lw      a0,0x0(t0)
        move   a0,t1
	bal	hexserial
        nop
	TTYDBG("\r\n")

        lw     t1,0x0(t0)
        addi   t0,t0,0x4

        //lw      a0,0x0(t0)
        move   a0,t1
	bal	hexserial
        nop
	TTYDBG("\r\n")

        bne	t0, t2, read
        nop
	b       endtest
	nop

        //li     t0,0xa0000100
        //lw     t2,0x0(t0)

display:
        move   a0,t0
        //lw      a0,0x0(t0)
	bal	hexserial
        nop
        move   a0,t1
        //lw      a0,0x0(t0)
	bal	hexserial
        nop
	TTYDBG("\r\n")

        b read
	nop

1:
        b 1b
        nop
#endif
/*

	li	t0, 0x80000000+1*1024*16//1024
	li	t1, 0x80000000
	li	t2, 0xffffffff
	
	sw	t2, 0(t1)
*/
endtest:

/* zhb */ 
#if 0 
	TTYDBG("Testing memory...\r\n")
	
	li	t7, 1
tmem:
#if 0//whd cache uncache test

/*
2:
	li	t0, 0xa0000000+1*1024*16//1024
	li	t1, 0xa0000000
	li	t2, 0xffffffff
	
1:
	sw	t2, 0(t1)
	addi	t1, 4
	
	bne	t1, t0, 1b
	nop

	TTYDBG("write uncache ok\r\n")
	
	li	t0, 0x80000000+1*1024*16//1024
	li	t1, 0x80000000
	li	t2, 0

2:	sw	t2, 0(t1)
	addi	t1, 4
	bne	t1, t0, 2b
	nop

	TTYDBG("write cache ok\r\n")
	
	li	t0, 0x80000000+1*1024*16//1024
	li	t1, 0x80000000
	li	t2, 0xffffffff

3:
	beq	t1, t0, 2f
	nop
	lw	a0, 0(t1)
	addi	t1, 4
	//beq	a0, t2, 3b
	beqz	a0, 3b
	nop
	
	bal	hexserial
	nop

	move	a0, t1
	bal	hexserial
	nop
	TTYDBG("\r\n")

	b	3b
	nop
2:
	li	t0, 0xa0000000+1*1024*18//1024
	li	t1, 0xa0000000
	li	t2, 0xffffffff
	
1:
	sw	t2, 0(t1)
	addi	t1, 4
	
	bne	t1, t0, 1b
	nop

	TTYDBG("write uncache ok,130\r\n")
	
	li	t0, 0x80000000+1*1024*18//1024
	li	t1, 0x80000000
	li	t2, 0

2:	sw	t2, 0(t1)
	addi	t1, 4
	bne	t1, t0, 2b
	nop

	TTYDBG("write cache ok\r\n")
	
	li	t0, 0x80000000+1*1024*18//1024
	li	t1, 0x80000000
	li	t2, 0xffffffff

3:
	beq	t1, t0, 2f
	nop
	lw	a0, 0(t1)
	addi	t1, 4
	//beq	a0, t2, 3b
	beqz	a0, 3b
	nop
	
	bal	hexserial
	nop

	move	a0, t1
	bal	hexserial
	nop
	TTYDBG("\r\n")

	b	3b
	nop
2:
	li	t0, 0xa0000000+1*1024*256//1024
	li	t1, 0xa0000000
	li	t2, 0xffffffff
	
1:
	sw	t2, 0(t1)
	addi	t1, 4
	
	bne	t1, t0, 1b
	nop

	TTYDBG("write uncache ok 256\r\n")
	
	li	t0, 0x80000000+1*1024*256//1024
	li	t1, 0x80000000
	li	t2, 0

2:	sw	t2, 0(t1)
	addi	t1, 4
	bne	t1, t0, 2b
	nop

	TTYDBG("write cache ok\r\n")
	
	li	t0, 0x80000000+1*1024*256//1024
	li	t1, 0x80000000
	li	t2, 0xffffffff

3:
	beq	t1, t0, 2f
	nop
	lw	a0, 0(t1)
	addi	t1, 4
	//beq	a0, t2, 3b
	beqz	a0, 3b
	nop
	
	bal	hexserial
	nop

	move	a0, t1
	bal	hexserial
	nop
	TTYDBG("\r\n")

	b	3b
	nop

*/
2:
	li	t0, 0xa0000000+1*1024*512//1024
	li	t1, 0xa0000000
	li	t2, 0xffffffff
	
1:
	sw	t2, 0(t1)
	addi	t1, 4
	
	bne	t1, t0, 1b
	nop

	TTYDBG("write uncache ok\r\n")
	
	li	t0, 0x80000000+1*1024*512//1024
	li	t1, 0x80000000
	li	t2, 0

2:	sw	t2, 0(t1)
	addi	t1, 4
	bne	t1, t0, 2b
	nop

	TTYDBG("write cache ok\r\n")
	
	li	t0, 0x80000000+1*1024*512//1024
	li	t1, 0x80000000
	li	t2, 0xffffffff

3:
	beq	t1, t0, 2f
	nop
	lw	a0, 0(t1)
	addi	t1, 4
	//beq	a0, t2, 3b
	beqz	a0, 3b
	nop
	
	bal	hexserial
	nop

	move	a0, t1
	bal	hexserial
	nop
	TTYDBG("\r\n")

	b	3b
	nop
	
	
	
	
	

#else
	//li	t0, 0x80000000+1*1024*1024
	//li	t1, 0x80000000
	li	t0, 0xa0000000+1*1024*1024
	li	t1, 0xa0000000
	li	t2, 0xffffffff
1:
	sw	t2, 0(t1)
	lw	t3, 0(t1)
	bne	t3, t2, 1f
	nop
	not	t2
	sw	t2, 0(t1)
	lw	t3, 0(t1)
	bne	t3, t2, 1f
	nop
	not	t2
	subu	t2, 1
	addu	t1, 4
	beq	t1, t0, 2f
	nop
	and	t4, t1, 0x000fffff
	bnez	t4, skipdot
	li	a0, '.'
	bal	tgt_putchar
	nop
skipdot:
	b	1b
	nop
#endif
1:
	TTYDBG("Memory test failed at ");
	move	a0,	t1
	bal	hexserial
	nop
	TTYDBG("\r\nWrite=");
	move	a0, t2
	bal	hexserial
	nop
	TTYDBG("\r\nRead=");
	move	a0, t3
	bal	hexserial
	nop
1:
	b	1b
	nop
2:
	TTYDBG("Testing ok...\r\n");
	sub	t7,1
	beqz	t7, 1f
	nop
	b	tmem
	nop
1:	
/*
	b	1b
	nop
*/

#endif


/* zhb */
#if 0
/*	li	t0, 0		*/
/*	li	t1, 0x90000	*/
/*1:	addiu	t0, 1		*/
/*	bne	t0, t1, 1b	*/
	nop

	TTYDBG("Testing memory...\r\n")
	
	li	t7, 10
tmem:
	li	t0, 0xa0000000+1*1024*1024
	li	t1, 0xa0000000
	li	t2, 0xffffffff
1:
	sw	t2, 0(t1)
	lw	t3, 0(t1)
	bne	t3, t2, 1f
	nop
	not	t2
	sw	t2, 0(t1)
	lw	t3, 0(t1)
	bne	t3, t2, 1f
	nop
	not	t2
	subu	t2, 1
	addu	t1, 4
	beq	t1, t0, 2f
	nop
	and	t4, t1, 0x000fffff
	bnez	t4, skipdot
	li	a0, '.'
	bal	tgt_putchar
	nop
skipdot:
	b	1b
	nop
1:
	TTYDBG("Memory test failed at ");
	move	a0,	t1
	bal	hexserial
	nop
	TTYDBG("\r\nWrite=");
	move	a0, t2
	bal	hexserial
	nop
	TTYDBG("\r\nRead=");
	move	a0, t3
	bal	hexserial
	nop
1:
	b	1b
	nop
2:
	TTYDBG("Testing ok...\r\n");
	sub	t7,1
	beqz	t7, 1f
	nop
	b	tmem
	nop
1:	
	b	1b
	nop
#endif


#if 1 /* Read adustment */
//CACHE WRITE whd
	    TTYDBG("Write \r\n")
        li      t0, 0x80000400
        li      a0, 0x0
        li      t2, 0x80000420

        li      a0, 0x99999999
        sw      a0, 0x0(t0)
        li      a0, 0x99999999
        sw      a0, 0x4(t0)
        li      a0, 0x66666666
        sw      a0, 0x8(t0)
        li      a0, 0x66666666
        sw      a0, 0xc(t0)

        li      a0, 0x55555555
        sw      a0, 0x10(t0)
        li      a0, 0x55555555
        sw      a0, 0x14(t0)
        li      a0, 0xaaaaaaaa
        sw      a0, 0x18(t0)
        li      a0, 0xaaaaaaaa
        sw      a0, 0x1c(t0)

        
        li      t0, 0x80000400
        cache   23, 0x0(t0) //HitWBInvalidate_S

        dli     t0, 0
        dli     t1, 0xff
        li      v0, 0
        li      v1, 0

//SLICE 0: Set Delay of DQS
set_0:
        li      t2, 0xaff001f0
        ld      a0, 0x0(t2)
        li      a1, 0xffff00ffffffffff
        and     a0, a0, a1
        dsll    a1, t0, 40
        or      a0, a0, a1
        sd      a0, 0x0(t2)

	    TTYDBG("\r\nLOOP until value change done.\r\n")
        li      t1, 0xfffffff #360Mhz
1:
        bnez    t1, 1b
        addi    t1, t1, -1
    
	    TTYDBG("LOOP done.\r\n")

  //Test DQS

read_right_0:
        li      t2, 0xa0000400
        lw      a0, 0x0(t2)
        li      a1, 0x000000ff
        and     a0, a0, a1
        li      a1, 0x99

        daddi   t0, t0, 1

        bne     a0, a1, set_0
        nop

        lw      a0, 0x10(t2)
        li      a1, 0x000000ff
        and     a0, a0, a1
        li      a1, 0x55
        bne     a0, a1, set_0
        nop

        addi    v0, t0, -1
        move    a0, v0
        bal     hexserial
        nop
        bne     t0, t1, set_0
        nop

#endif

/*
////////////////////
        li      t2, 0xaff00240
        ld      a0, 0x0(t2)
        li      a1, 0xffffffffffff00ff
        and     a0, a0, a1
        sd      a0, 0x0(t2)
*/
