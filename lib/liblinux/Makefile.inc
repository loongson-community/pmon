LINUXLIBDIR=	${S}/lib/liblinux

LINUXLIBDST?=	${OBJDIR}/lib/liblinux

LINUXLIB=	${ZLIBDST}/liblinux.a

export LINUXLIB LINUXLIBDIR LINUXLIBDST
${LINUXLIB}: 
	make -C ${LINUXLIBDIR}

LINUXLIBDIR=	${S}/lib/liblinux

LINUXLIBDST?=	${OBJDIR}/lib/liblinux

LINUXLIB=	${ZLIBDST}/liblinux.a

export LINUXLIB LINUXLIBDIR LINUXLIBDST
${LINUXLIB}: 
	make -C ${LINUXLIBDIR}
