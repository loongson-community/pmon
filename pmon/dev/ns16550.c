/* $Id: ns16550.c,v 1.1.1.1 2006/06/29 06:43:25 cpu Exp $ */

/*
 * Copyright (c) 2000-2002 Opsycon AB  (www.opsycon.se)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by
 *	Opsycon Open System Consulting AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <termio.h>
#include <machine/pio.h>
#include <machine/bus.h>
#include <pmon.h>
#include <pmon/dev/ns16550.h>

extern int getbaudval __P((int));

static int nsinit(volatile ns16550dev * dp)
{
#ifdef USE_NS16550_FIFO
	unsigned int x;
	static int nsfifo;

	/* force access to id reg */
	outb(&dp->cfcr, 0);
	outb(&dp->iir, 0);
	if ((inb(&dp->iir) & 0x38) != 0) {
		return 1;
	}

	/* in case it really is a 16550, enable the fifos */
	outb(&dp->fifo, FIFO_ENABLE);
	for (x = 0; x < 1000000; x++) ;
	outb(&dp->fifo,
	     FIFO_ENABLE | FIFO_RCV_RST | FIFO_XMT_RST | FIFO_TRIGGER_1);
	for (x = 0; x < 1000000; x++) ;

	if ((inb(&dp->iir) & IIR_FIFO_MASK) == IIR_FIFO_MASK)
		nsfifo = 1;
	else {
		dp->fifo = 0;
	}
#endif				/* NS16650_FIFO */
	return 0;
}

static int nsprogram(volatile ns16550dev * dp, unsigned long freq, int baudrate)
{
	static int rates[] = {
		50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400,
		4800, 9600, 19200, 38400, 57600, 115200, 0
	};

	unsigned short brtc;
	int timeout, *p;

	/* wait for Tx fifo to completely drain */
	timeout = 1000000;
	while (!(inb(&dp->lsr) & LSR_TSRE)) {
		if (--timeout == 0)
			break;
	}

	baudrate = getbaudval(baudrate);
	for (p = rates; *p; p++)
		if (*p == baudrate)
			break;
	if (*p == 0)
		return 1;

	brtc = (freq) / (16 * (baudrate));

	outb(&dp->cfcr, CFCR_DLAB);
	outb(&dp->data, brtc & 0xff);
	outb(&dp->ier, brtc >> 8);
	outb(&dp->cfcr, CFCR_8BITS);
	outb(&dp->mcr, MCR_IENABLE | MCR_DTR | MCR_RTS);
	outb(&dp->ier, 0);
	return 0;
}


int nsprobe(volatile ns16550dev *dp)
{     
    char tmp;
    outb(&dp->ier,0x00);
    outb(&dp->ier,0xf);
    tmp = inb(&dp->ier);
    if (tmp == 0xf)
    {    
        outb(&dp->ier,0x00);
        return UART_EXISTED;
    }else {
        return UART_NOTEXISTED;
    }            
}

int ns16550(int op, struct DevEntry *dev, unsigned long param, int data)
{
	volatile ns16550dev *dp;

	dp = (ns16550dev *) dev->sio;

	switch (op) {
    case OP_PROBE:
        return nsprobe(dp);    
    case OP_INIT:
		return nsinit(dp);

	case OP_XBAUD:
	case OP_BAUD:
		return nsprogram(dp, dev->freq, data);

	case OP_TXRDY:
		return (inb(&dp->lsr) & LSR_TXRDY);

	case OP_TX:
		outb(&dp->data, data);
		break;

	case OP_RXRDY:
		return (inb(&dp->lsr) & LSR_RXRDY);

	case OP_RX:
		return inb(&dp->data) & 0xff;

	case OP_RXSTOP:
		if (data)
			outb(&dp->mcr, inb(&dp->mcr) & ~MCR_RTS);
		else
			outb(&dp->mcr, inb(&dp->mcr) | MCR_RTS);
		break;
	}
	return 0;
}
