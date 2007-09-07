/*
 * arch/mips/boot/common/misc-common.c
 * 
 * Misc. bootloader code (almost) all platforms can use
 *
 * Author: Johnnie Peters <jpeters@mvista.com>
 * Editor: Tom Rini <trini@mvista.com>
 *
 * Derived from arch/ppc/boot/prep/misc.c
 *
 * Ported by Pete Popov <ppopov@mvista.com> to
 * support mips board(s).  I also got rid of the vga console
 * code.
 *
 * Copyright 2000-2001 MontaVista Software Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR   IMPLIED
 * WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT,  INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 * USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "zlib.h"

char *avail_ram;
char *end_avail;
extern char _end[];
extern char _edata[];

extern void flash_gpio(int);
extern void serial_putc(const char c);
void puts(const char *);
void puthex(unsigned long val);
void gunzip(void *, int, unsigned char *, int *);
void decompress_pmon(unsigned long load_addr, int num_words);

#define CACHED_MEMORY_ADDR	0x80000000
#define UNCACHED_MEMORY_ADDR	0xa0000000

#define	CACHED_TO_PHYS(x)	((unsigned)(x) & 0x1fffffff)
#define	PHYS_TO_UNCACHED(x) 	((unsigned)(x) | UNCACHED_MEMORY_ADDR)
#define	CACHED_TO_UNCACHED(x)	(PHYS_TO_UNCACHED(CACHED_TO_PHYS(x)))


void halt(void)
{
	puts("-- System halted --\n");
	while(1); 
}

void
tgt_reboot()
{

	void (*longreach) (void);
	
	longreach = (void *)0xbfc00000;
	(*longreach)();
	while(1);
}

void puts(const char *s)
{
	char c;
	while ( ( c = *s++ ) != '\0' ) {
	        serial_putc(c);
	        if ( c == '\n' ) serial_putc('\r');
	}
}


char *zimage_start;
int zimage_size;

void decompress_pmon(unsigned long load_addr, int num_words)
{
	/*
	 * Reveal where we were loaded at and where we
	 * were relocated to.
	 */
	 
	char *greeting="Welcoming to use PMON For ITE-GODSON2\n";
	puts(greeting);
	puts("loaded at:     "); puthex(load_addr); puts("\n");

	zimage_start = (char *)(0xbfc00000 + ZIMAGE_OFFSET);
	zimage_size = ZIMAGE_SIZE;

	avail_ram = (char *) ((((unsigned long)_end)+0xfff)&0xfffff000);
	puts("zimage at:     "); puthex((unsigned long)zimage_start);
	puts(" "); puthex((unsigned long)(zimage_size+zimage_start)); puts("\n");

	/* assume the chunk below 8M is free */
	avail_ram = (char *)AVAIL_RAM_START;
	end_avail = (char *)AVAIL_RAM_END;

	/* Display standard Linux/MIPS boot prompt for kernel args */
	puts("Uncompressing PMON...\n");
	/* I don't like this hard coded gunzip size (fixme) */
	gunzip((void *)LOADADDR, 0x400000, zimage_start, &zimage_size);
	puts("Enter PMON stage 2...\n");
}

void *zalloc(void *x, unsigned items, unsigned size)
{
	void *p = avail_ram;
	
	size *= items;
	size = (size + 7) & -8;
	avail_ram += size;
	if (avail_ram > end_avail) {
		puts("oops... out of memory\n");
		halt();
	}
	return p;
}

void zfree(void *x, void *addr, unsigned nb)
{
}

#define HEAD_CRC	2
#define EXTRA_FIELD	4
#define ORIG_NAME	8
#define COMMENT		0x10
#define RESERVED	0xe0

#define DEFLATED	8

void gunzip(void *dst, int dstlen, unsigned char *src, int *lenp)
{
	z_stream s;
	int r, i, flags;
	
	/* skip header */
	i = 10;
	flags = src[3];
	if (src[2] != DEFLATED || (flags & RESERVED) != 0) {
		puts("bad gzipped data\n");
		halt();
	}
	if ((flags & EXTRA_FIELD) != 0)
		i = 12 + src[10] + (src[11] << 8);
	if ((flags & ORIG_NAME) != 0)
		while (src[i++] != 0)
			;
	if ((flags & COMMENT) != 0)
		while (src[i++] != 0)
			;
	if ((flags & HEAD_CRC) != 0)
		i += 2;
	if (i >= *lenp) {
		puts("gunzip: ran out of data in header\n");
		halt();
	}
	
	s.zalloc = zalloc;
	s.zfree = zfree;
	r = inflateInit2(&s, -MAX_WBITS);
	if (r != Z_OK) {
		puts("inflateInit2 returned error\n");
		halt();
	}
	s.next_in = src + i;
	s.avail_in = *lenp - i;
	s.next_out = dst;
	s.avail_out = dstlen;
	r = inflate(&s, Z_FINISH);
	if (r != Z_OK && r != Z_STREAM_END) {
		puts("inflate returned error\n");
		halt();
	}
	*lenp = s.next_out - (unsigned char *) dst;
	inflateEnd(&s);
}

void
puthex(unsigned long val)
{

	unsigned char buf[10];
	int i;
	for (i = 7;  i >= 0;  i--)
	{
		buf[i] = "0123456789ABCDEF"[val & 0x0F];
		val >>= 4;
	}
	buf[8] = '\0';
	puts(buf);
}

