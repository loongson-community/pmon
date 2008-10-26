/*
 * BRIEF MODULE DESCRIPTION
 *	Simple ite8172 uart routines.
 *
 * Copyright 2001 MontaVista Software Inc.
 * Author: MontaVista Software, Inc.
 *		ppopov@mvista.com or source@mvista.com
 *
 *  This program is free software; you can redistribute	 it and/or modify it
 *  under  the terms of	 the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the	License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED	  ``AS	IS'' AND   ANY	EXPRESS OR IMPLIED
 *  WARRANTIES,	  INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO	EVENT  SHALL   THE AUTHOR  BE	 LIABLE FOR ANY	  DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED	  TO, PROCUREMENT OF  SUBSTITUTE GOODS	OR SERVICES; LOSS OF
 *  USE, DATA,	OR PROFITS; OR	BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN	 CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */
//#include <linux/config.h>
//#include <asm/io.h>
//#include "ns16550.h"


#define 	UART_BASE	0xb4011800
#define		UART_IER	0x01
#define 	UART_LSR	0x05
#define		UART_LCR	0x03
#define 	UART_DLL	0
#define		UART_DLM	0x01
#define		UART_FCR	0x02
#define 	UART_TX		0
#define		UART_RX		0
#define		LSR_DR		0x01
#if 0
#define         UART16550_BAUD_2400             2400
#define         UART16550_BAUD_4800             4800
#define         UART16550_BAUD_9600             9600
#define         UART16550_BAUD_19200            19200
#define         UART16550_BAUD_38400            38400
#define         UART16550_BAUD_57600            57600
#define         UART16550_BAUD_115200           115200

#define         UART16550_PARITY_NONE           0
#define         UART16550_PARITY_ODD            0x08
#define         UART16550_PARITY_EVEN           0x18
#define         UART16550_PARITY_MARK           0x28
#define         UART16550_PARITY_SPACE          0x38

#define         UART16550_DATA_5BIT             0x0
#define         UART16550_DATA_6BIT             0x1
#define         UART16550_DATA_7BIT             0x2
#define         UART16550_DATA_8BIT             0x3

#define         UART16550_STOP_1BIT             0x0
#define         UART16550_STOP_2BIT             0x4
#endif

/* memory-mapped read/write of the port */
#define UART16550_READ(y)    ((*((volatile unsigned char *)(UART_BASE + y)) & 0xff))
#define UART16550_WRITE(y,z) ( *((volatile unsigned char *)(UART_BASE + y))=((z)&0xff) )

int initial = 0;
/*
 * We use uart 0, which is already initialized by
 * the bios. 
 */
void *
serial_init(void)
{
	*(volatile unsigned short *)(0xb4015800) &= ~0x0100;
	UART16550_WRITE(UART_LCR, 0x80);
	UART16550_WRITE(UART_DLL, 0x01);
	UART16550_WRITE(UART_DLM, 0);
	UART16550_WRITE(UART_LCR, 0x03);
	UART16550_WRITE(UART_FCR, 0x47);
	UART16550_WRITE(UART_IER, 0);
}

void
serial_putc(unsigned char c)
{
	if (initial == 0) {
		serial_init();
		initial = 1;
	}
	while ((UART16550_READ(UART_LSR)&0x40) == 0);
	UART16550_WRITE(UART_TX, c);
	return ;
}

unsigned char
serial_getc(void)
{
	if (initial == 0) {
		serial_init();
		initial = 1;
	}
	while((UART16550_READ(UART_LSR) & 0x1) == 0);
	return UART16550_READ(UART_RX);
}

int
serial_tstc(void)
{
	return ((UART16550_READ(UART_LSR) & LSR_DR) != 0);
}
