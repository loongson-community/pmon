
#include <asm.h>
#include <regdef.h>
#include <target/bonito.h>

	.data
.global cachelock_start;
.global cachelock_end;
cachelock_start:
LEAF(tgt_putchar1)
	la	v0, COM1_BASE_ADDR
1:
	lbu	v1, 5(v0)
	and	v1, 0x20
	beqz	v1, 1b
	nop

	sb	a0, 0(v0)
	j	ra
	nop	
END(tgt_putchar1)

LEAF(tgt_testchar1)
	.set noat
        la      v0, COM1_BASE_ADDR
1:
        lbu     v1, 5(v0)
        and     v0, v1, 1
        jr	ra
        nop
		.set at
END(tgt_testchar)

#define SPI_BASE 0xbfe00220
#define SPCR      0x0 
#define SPSR      0x1 
#define TXFIFO    0x2 
#define SPER      0x3 
#define PARAM1     0x4 
#define SOFTCS    0x5 
#define PARAM2    0x6 

.macro set_spi add,val
li v1,\val;
sb v1,\add(v0);
.endm


LEAF(spi_init)

li v0,SPI_BASE 
set_spi SPSR,0xc0;
set_spi PARAM1,0x10;
set_spi SPER,0x5;
set_spi PARAM2,1;
set_spi SPCR,0x50;
jr ra
nop
END(spi_init)


LEAF(program)
.set mips32
.set noreorder
.set noat
#spi io base
li t0,SPI_BASE;
move t3,ra
li v0,0x11; /*high cs*/
sb v0,5(t0);
bal spi_waitsr; /*wait_sr(v0)*/
nop;
li v0,0x1;
sb v0,5(t0);
li v0,6;
bal spi_wb;
nop;
li v0,0x11;
sb v0,5(t0);
li v0,0x1;
sb v0,5(t0);
li a3,0x2;
300:
li v0,0x05;
bal spi_wb;
nop;
andi v0,0x3;
bne v0,a3,300b;
nop;
li v0,0x11;
sb v0,5(t0);
li v0,0x1;
sb v0,5(t0);
li v0,0x1;
bal spi_wb;
nop;
li v0,0x0;
bal spi_wb;
nop;
li v0,0x11;
sb v0,5(t0);
1:
lw t2,(a0)
12:
bal spi_waitsr;nop;
li v0,0x1;
sb v0,5(t0);
li v0,6;
bal spi_wb;
nop;
li v0,0x11;
sb v0,5(t0);
li v0,0x1;
sb v0,5(t0);
li a3,0x2;
300:li v0,0x05;
bal spi_wb;
nop;
andi v0,0x3;
bne v0,a3,300b;
nop;
li v0,0x11;
sb v0,5(t0);
li v0,0x1;
sb v0,5(t0);
li v0,2;
bal spi_wb;
nop;
and v0,a1,0xfff;
bnez v0,11f;
nop;
11:srl v0,a1,16;
bal spi_wb;
nop;
srl v0,a1,8;
bal spi_wb;
nop;
move v0,a1;
bal spi_wb;
nop;
2:
andi v0,t2,0xff
bal spi_wb;
nop;
srl t2, 8

li v0,0x11;
sb v0,5(t0);
addiu a0,1;
addiu a1,1;
addiu a2,-1;
beqz a2,3f;
nop;
andi v0, a0,3
bnez v0,12b
nop
b 1b;
nop;
3:li v0,0x11;
sb v0,5(t0); /*high cs*/
bal spi_waitsr;
nop;
sync;
move ra,t3;
jr ra ;
nop;
END(program)

LEAF(erase_area)
li t0,SPI_BASE; 
move t1, ra;
li v0,0x11;
sb v0,5(t0); /*high cs*/
bal spi_waitsr;   /*wait_sr(v0)*/
nop;
2:li v0,0x1;
sb v0,5(t0); /*low cs*/
li v0,6;  /* write enable */
bal spi_wb; 
nop; 
li v0,0x11;
sb v0,5(t0); /*high cs*/
li v0,0x1;
sb v0,5(t0); /*low cs*/
li a3,0x2;
300:li v0,0x05;  /* check WEL */
bal spi_wb; 
nop;
andi v0,0x3;
bne v0,a3,300b;
nop;
li v0,0x11;
sb v0,5(t0); /*high cs*/
li v0,0x1;
sb v0,5(t0); /*low cs*/
li v0,0x1;  /* write enable */
bal spi_wb; 
nop; 
li v0,0x0;  /* write sr to 0 */
bal spi_wb; 
nop; 
li v0,0x11;
sb v0,5(t0); /*high cs*/
bal spi_waitsr;   /*wait_sr(v0)*/
nop;
li v0,0x1;
sb v0,5(t0); /*low cs*/
li v0,6;  /* write enable */
bal spi_wb; 
nop; 
li v0,0x11;
sb v0,5(t0); /*high cs*/
li v0,0x1;
sb v0,5(t0); /*low cs*/
li a3,0x2;
300:li v0,0x05;  /* check WEL */
bal spi_wb; 
nop;
andi v0,0x3;
bne v0,a3,300b;
nop;
li v0,0x11;
sb v0,5(t0); /*high cs*/
li v0,0x1;
sb v0,5(t0); /*low cs*/
li v0,0xd8;  /*bluk erase*/
bal spi_wb; 
nop; 
srl v0,a0,16; /*addr*/
bal spi_wb; 
nop; 
srl v0,a0,8; 
bal spi_wb; 
nop; 
move v0,a0; 
bal spi_wb; 
nop; 
li v0,0x11;
sb v0,5(t0); /*high cs*/
bal spi_waitsr;   /*wait_sr(v0)*/
nop;
addu a0,a2;
slt v0,a1,a0;
beqz v0,2b;
nop;
sync;
move ra,t1
jr ra
nop; 
END(erase_area)

LEAF(erase)
li t0,SPI_BASE; 
move t1, ra;
li v0,0x11;
sb v0,5(t0); /*high cs*/
bal spi_waitsr;   /*wait_sr(v0)*/
nop;
2:li v0,0x1;
sb v0,5(t0); /*low cs*/
li v0,6;  /* write enable */
bal spi_wb; 
nop; 
li v0,0x11;
sb v0,5(t0); /*high cs*/
li v0,0x1;
sb v0,5(t0); /*low cs*/
li a3,0x2;
300:li v0,0x05;  /* check WEL */
bal spi_wb; 
nop;
andi v0,0x3;
bne v0,a3,300b;
nop;
li v0,0x11;
sb v0,5(t0); /*high cs*/
li v0,0x1;
sb v0,5(t0); /*low cs*/
li v0,0x1;  /* write enable */
bal spi_wb; 
nop; 
li v0,0x0;  /* write sr to 0 */
bal spi_wb; 
nop; 
li v0,0x11;
sb v0,5(t0); /*high cs*/
bal spi_waitsr;   /*wait_sr(v0)*/
nop;
li v0,0x1;
sb v0,5(t0); /*low cs*/
li v0,6;  /* write enable */
bal spi_wb; 
nop; 
li v0,0x11;
sb v0,5(t0); /*high cs*/
li v0,0x1;
sb v0,5(t0); /*low cs*/
li a3,0x2;
300:li v0,0x05;  /* check WEL */
bal spi_wb; 
nop;
andi v0,0x3;
bne v0,a3,300b;
nop;
li v0,0x11;
sb v0,5(t0); /*high cs*/
li v0,0x1;
sb v0,5(t0); /*low cs*/
li v0,0xc7;  /*chip erase*/
bal spi_wb; 
nop; 
li v0,0x11;
sb v0,5(t0); /*high cs*/
bal spi_waitsr;   /*wait_sr(v0)*/
nop;
sync;
move ra,t1
jr ra
nop; 
END(erase)

LEAF(spi_wb)
sb v0,2(t0); /****  103 send_cmd(v0) *****/
1:lb v0,1(t0); 
andi v0,1; 
bnez v0,1b;
nop;
lb v0,2(t0); 
jr ra; 
nop; /****  103 send_cmd(v0)*****/
END(spi_wb)

LEAF(spi_waitsr)
move a3,ra; /****  112 wait_sr(v0)*****/
li v0,0x1;
sb v0,5(t0); /*low cs*/
li v0,0x5;
bal spi_wb;
nop;
1120:li  v0,0xff;
bal  spi_wb;
nop;
andi v0,1;
bnez v0,1120b;
nop;
li v0,0x11;
sb v0,5(t0); /*high cs*/
move ra, a3;
jr ra
nop; /****  112 wait_sr(v0)****/
END(spi_waitsr)


cachelock_end:

	.text

.set mips3;
initmips:
.global initmips;
     /*for data and bss*/
     dli    t0,0x900000003ff00200 //lock 0x9f000000 - 0x9f00f000
     dli    t1,0xffffffffffffc000
     sd     t1,0x40(t0)
     la t1, _fdata
     li v0, 0x1fffffff
     and t1, v0
     dli v0,0x8000000000000000
     or t1,v0
     sd     t1,0x0(t0)


	la sp, _fdata
	addiu sp, 0x4000-16

	/* Clear BSS */
	la	t0, _edata
	la	t2, _end
2:	sw	zero, 0(t0)
	bne	t2, t0, 2b
	addu	t0, 4

	la	t0, _fdata
	addiu	t2,t0,0x4000
2:	lw	zero, 0(t0)
	bne	t2, t0, 2b
	addu	t0, 4

#if 1
bal tgt_testchar
nop
beqz v0,1f
nop
bal tgt_getchar
nop
li v1,'d';
bne v0,v1,1f
nop
b 2f
nop
1:
b initmips1
nop
#endif
2:

	bal xmodem
	nop
