
    ###Enable the reg space###
#if 1
	TTYDBG	("\r\nEnable register space of MEMORY\r\n")
        li  t2,0xbfe00180
        lw  a1, 0x0(t2)
        li  a0, 0xfffffeff
        and a1,a1,a0
        sw  a1,0x0(t2)
#endif    


	li	msize,0x0f000000
//#define MEMORY_512x2
#define MEMORY_1G_MC1

	    PRINTSTR("MC0 High DDR2 space open : 0x20000000 - 0x2FFFFFFF\r\n")
        dli     t0, 0x900000003ff00010  #base
        dli     t1, 0x20000000
        sd      t1, 0(t0)
        sync    

        dli     t0, 0x900000003ff00050  #mask
        dli     t1, 0xfffffffff0000000
        sd      t1, 0(t0)
        sync    

        dli     t0, 0x900000003ff00090  #mmap
        dli     t1, 0x100000f0
        sd      t1, 0(t0)
        sync

        #dli a0, 0xaff00000
#ifdef MEMORY_1G_MC1 //set memory space of MC0 to MC1
	    PRINTSTR("MC1 USING MC0 space\r\n")
	    PRINTSTR("MC1 DDR2 config begin\r\n")

        dli     t0, 0x900000003ff00080  #mmap of Windows 0
        dli     t1, 0xf1
        sd      t1, 0(t0)
        sync

        dli     t0, 0x900000003ff00090  #mmap of Windows 2
        dli     t1, 0x100000f1
        sd      t1, 0(t0)
        sync

        dli     a0, 0x900000000ff00000
	    bal	    ddr2_config_mc1
	    nop
	    PRINTSTR("MC1 DDR2 config end\r\n")
#else
	    PRINTSTR("MC0 DDR2 config begin\r\n")
        dli     a0, 0x900000000ff00000
	    bal	    ddr2_config
	    nop
	    PRINTSTR("MC0 DDR2 config end\r\n")
#endif

#ifdef MEMORY_512x2 /* initialize win2 to config the second ddr controller*/
	    PRINTSTR("MC1 DDR2 space open : 0x40000000 - 0x5FFFFFFF\r\n")
        dli     t0, 0x900000003ff00018  #base
        dli     t1, 0x40000000
        sd      t1, 0(t0)
        sync    

        dli     t0, 0x900000003ff00058  #mask
        dli     t1, 0xffffffffe0000000
        sd      t1, 0(t0)
        sync    

        dli     t0, 0x900000003ff00098  #mmap
        dli     t1, 0xf1
        sd      t1, 0(t0)
        sync

	    PRINTSTR("MC1 DDR2 config begin\r\n")
        dli     a0, 0x900000004ff00000
	    bal	    ddr2_config_mc1
	    nop

	    PRINTSTR("MC1 DDR2 config end\r\n")
#else /* MEMORY_1G_MC1 */
	    PRINTSTR("MC1 HIGH DDR2 space open : 0x40000000 - 0x5FFFFFFF\r\n")
        dli     t0, 0x900000003ff00018  #base
        dli     t1, 0x40000000
        sd      t1, 0(t0)
        sync    

        dli     t0, 0x900000003ff00058  #mask
        dli     t1, 0xffffffffe0000000
        sd      t1, 0(t0)
        sync    

        dli     t0, 0x900000003ff00098  #mmap
        dli     t1, 0x200000f1
        sd      t1, 0(t0)
        sync
#endif

#if 0 //print registers
    li      t1, 152 ##0x72
    li      t2, 0xaff00000

reg_read:
    ld      a0, 0x0(t2)

    bal	    hexserial
    nop
    PRINTSTR("\r\n")

    addiu   t2, t2, 16
    bnez    t1, reg_read
    addiu   t1, t1, -1

#endif


#if 0 /* read ddr2 registers */
        li	t0, 0xaff00000
        
not_locked:
        ld	t1, 0x10(t0)
        andi    t1, 0x01
        beqz    t1, not_locked
        nop

        PRINTSTR("DDR2 DLL locked\r\n")
        
        ld	t1, 0xf0(t0)
        move    a0, t1
        bal     hexserial
        nop

#endif
        
    ###disable the reg space###
#if 1
	TTYDBG	("Disable register space of MEMORY\r\n")
        li  t2,0xbfe00180
        lw  a1,0x0(t2)
        or  a1,a1,0x100
        sw  a1,0x0(t2)
#endif    

#if 1 // AdonWang disable ddr3 readbuff
       TTYDBG("Disable read buffer\r\n")
       li      t0,0xbfe00180
       //dli     t1,0x1ffffff617   
       //dli     t1,0x07fffff617   
       //dli     t2,0x4000000000
       //or      t1,t1,t2
       lw      t1, 0x4(t0)
        li      a0, 0x18
        or      t1, t1, a0
       sw      t1,0x4(t0)
#endif

#if 1 // AdonWang disable cpu buffered read
/* !!!!!!!!!! IMPORTANT !!!!!!!!!! */
       TTYDBG("Disable cpu buffered read\r\n")
       li      t0,0xbfe00180
       lw      t1, 0x0(t0)
       li      a0, 0xfffffdff
       and     t1, t1, a0
       sw      t1, 0x0(t0)
#endif

