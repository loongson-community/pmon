#	$Id: Makefile,v 1.1.1.1 2006/06/29 06:43:25 cpu Exp $

PROG=	bootelf

SRCS=	bootelf.c 
DESTDIR=/opt/pmon2000/tools

OBJS=	bootelf.o

CFLAGS+=-I${.CURDIR} -I.
#CFLAGS+=-I${.CURDIR} -I. -DDO_SWAP

${PROG}: ${OBJS}
	${CC} -o $@ ${OBJS} ${LIBS}

install:
	cp ${PROG} ${DESTDIR}/bin

clean:
	rm -rf ${OBJS} ${PROG}
