#	$Id: Makefile,v 1.1.1.1 2006/09/14 01:59:06 root Exp $ */
#
#
# Copyright (c) 2001 Opsycon AB  (www.opsycon.se)
# Copyright (c) 2002 Patrik Lindergren  (www.lindergren.com)
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#	This product includes software developed by Opsycon AB, Sweden.
#	This product includes software developed by Patrik Lindergren.
# 4. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#

LIB=z
NOPIC=

MACHINE=	${XMACHINE}
MACHINE_ARCH=	${XMACHINE_ARCH}
CURDIR=$(shell pwd)

M=	${CURDIR}/arch/${MACHINE_ARCH}

CPPFLAGS=	-I$M ${ZLIBCPPFLAGS} -U_KERNEL

VPATH+=	${M} ${ZLIBDIR}
 
include ${M}/Makefile.inc

# Files to clean up
CLEANFILES+= ${OBJDIR}/lib${LIB}.a

include ${PMONDIR}/tools/scripts/pmon.lib.gmk

${OBJDIR}/lib${LIB}.a: ${OBJS}
	@echo building standard ${LIB} library
	@rm -f $@
	@${AR} cq $@ ${OBJS}
	${RANLIB} $@
# DO NOT DELETE

adler32.o: zlib.h zconf.h
crc32.o: zlib.h zconf.h
infblock.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infblock.o: /mnt/sda8/git-source/pmon/include/string.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infblock.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infblock.o: infcodes.h infutil.h
infcodes.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infcodes.o: /mnt/sda8/git-source/pmon/include/string.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
infcodes.o: infcodes.h infutil.h inffast.h
inffast.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inffast.o: /mnt/sda8/git-source/pmon/include/string.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inffast.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
inffast.o: infcodes.h infutil.h inffast.h
inflate.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inflate.o: /mnt/sda8/git-source/pmon/include/string.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inflate.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h
inftrees.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inftrees.o: /mnt/sda8/git-source/pmon/include/string.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h inffixed.h
infutil.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infutil.o: /mnt/sda8/git-source/pmon/include/string.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infutil.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infutil.o: infcodes.h infutil.h

adler32.o: zlib.h zconf.h
crc32.o: zlib.h zconf.h
infblock.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infblock.o: /mnt/sda8/git-source/pmon/include/string.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infblock.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infblock.o: infcodes.h infutil.h
infcodes.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infcodes.o: /mnt/sda8/git-source/pmon/include/string.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
infcodes.o: infcodes.h infutil.h inffast.h
inffast.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inffast.o: /mnt/sda8/git-source/pmon/include/string.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inffast.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
inffast.o: infcodes.h infutil.h inffast.h
inflate.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inflate.o: /mnt/sda8/git-source/pmon/include/string.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inflate.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h
inftrees.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inftrees.o: /mnt/sda8/git-source/pmon/include/string.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h inffixed.h
infutil.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infutil.o: /mnt/sda8/git-source/pmon/include/string.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infutil.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infutil.o: infcodes.h infutil.h

adler32.o: zlib.h zconf.h
crc32.o: zlib.h zconf.h
infblock.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infblock.o: /mnt/sda8/git-source/pmon/include/string.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infblock.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infblock.o: infcodes.h infutil.h
infcodes.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infcodes.o: /mnt/sda8/git-source/pmon/include/string.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
infcodes.o: infcodes.h infutil.h inffast.h
inffast.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inffast.o: /mnt/sda8/git-source/pmon/include/string.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inffast.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
inffast.o: infcodes.h infutil.h inffast.h
inflate.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inflate.o: /mnt/sda8/git-source/pmon/include/string.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inflate.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h
inftrees.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inftrees.o: /mnt/sda8/git-source/pmon/include/string.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h inffixed.h
infutil.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infutil.o: /mnt/sda8/git-source/pmon/include/string.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infutil.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infutil.o: infcodes.h infutil.h

adler32.o: zlib.h zconf.h
crc32.o: zlib.h zconf.h
infblock.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/ansi.h
infblock.o: /mnt/sda8/git-source/pmon/include/string.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/endian.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infblock.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infblock.o: infcodes.h infutil.h
infcodes.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/ansi.h
infcodes.o: /mnt/sda8/git-source/pmon/include/string.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
infcodes.o: infcodes.h infutil.h inffast.h
inffast.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/ansi.h
inffast.o: /mnt/sda8/git-source/pmon/include/string.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/endian.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inffast.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
inffast.o: infcodes.h infutil.h inffast.h
inflate.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/ansi.h
inflate.o: /mnt/sda8/git-source/pmon/include/string.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/endian.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inflate.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h
inftrees.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/ansi.h
inftrees.o: /mnt/sda8/git-source/pmon/include/string.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h inffixed.h
infutil.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/ansi.h
infutil.o: /mnt/sda8/git-source/pmon/include/string.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito/machine/endian.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infutil.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infutil.o: infcodes.h infutil.h

adler32.o: zlib.h zconf.h
crc32.o: zlib.h zconf.h
infblock.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infblock.o: /mnt/sda8/git-source/pmon/include/string.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infblock.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infblock.o: infcodes.h infutil.h
infcodes.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infcodes.o: /mnt/sda8/git-source/pmon/include/string.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
infcodes.o: infcodes.h infutil.h inffast.h
inffast.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inffast.o: /mnt/sda8/git-source/pmon/include/string.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inffast.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
inffast.o: infcodes.h infutil.h inffast.h
inflate.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inflate.o: /mnt/sda8/git-source/pmon/include/string.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inflate.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h
inftrees.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inftrees.o: /mnt/sda8/git-source/pmon/include/string.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h inffixed.h
infutil.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infutil.o: /mnt/sda8/git-source/pmon/include/string.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infutil.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infutil.o: infcodes.h infutil.h

adler32.o: zlib.h zconf.h
crc32.o: zlib.h zconf.h
infblock.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infblock.o: /mnt/sda8/git-source/pmon/include/string.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infblock.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infblock.o: infcodes.h infutil.h
infcodes.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infcodes.o: /mnt/sda8/git-source/pmon/include/string.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
infcodes.o: infcodes.h infutil.h inffast.h
inffast.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inffast.o: /mnt/sda8/git-source/pmon/include/string.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inffast.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
inffast.o: infcodes.h infutil.h inffast.h
inflate.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inflate.o: /mnt/sda8/git-source/pmon/include/string.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inflate.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h
inftrees.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inftrees.o: /mnt/sda8/git-source/pmon/include/string.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h inffixed.h
infutil.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infutil.o: /mnt/sda8/git-source/pmon/include/string.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infutil.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infutil.o: infcodes.h infutil.h

adler32.o: zlib.h zconf.h
crc32.o: zlib.h zconf.h
infblock.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infblock.o: /mnt/sda8/git-source/pmon/include/string.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infblock.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infblock.o: infcodes.h infutil.h
infcodes.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infcodes.o: /mnt/sda8/git-source/pmon/include/string.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
infcodes.o: infcodes.h infutil.h inffast.h
inffast.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inffast.o: /mnt/sda8/git-source/pmon/include/string.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inffast.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
inffast.o: infcodes.h infutil.h inffast.h
inflate.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inflate.o: /mnt/sda8/git-source/pmon/include/string.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inflate.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h
inftrees.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inftrees.o: /mnt/sda8/git-source/pmon/include/string.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h inffixed.h
infutil.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infutil.o: /mnt/sda8/git-source/pmon/include/string.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infutil.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infutil.o: infcodes.h infutil.h

adler32.o: zlib.h zconf.h
crc32.o: zlib.h zconf.h
infblock.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infblock.o: /mnt/sda8/git-source/pmon/include/string.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infblock.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infblock.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infblock.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infblock.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infblock.o: infcodes.h infutil.h
infcodes.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infcodes.o: /mnt/sda8/git-source/pmon/include/string.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infcodes.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infcodes.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infcodes.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
infcodes.o: infcodes.h infutil.h inffast.h
inffast.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inffast.o: /mnt/sda8/git-source/pmon/include/string.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inffast.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inffast.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inffast.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inffast.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h infblock.h
inffast.o: infcodes.h infutil.h inffast.h
inflate.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inflate.o: /mnt/sda8/git-source/pmon/include/string.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inflate.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inflate.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inflate.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inflate.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h
inftrees.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
inftrees.o: /mnt/sda8/git-source/pmon/include/string.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
inftrees.o: /mnt/sda8/git-source/pmon/include/stdlib.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
inftrees.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
inftrees.o: /mnt/sda8/git-source/pmon/include/errno.h inftrees.h inffixed.h
infutil.o: zutil.h zlib.h zconf.h /mnt/sda8/git-source/pmon/include/stddef.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/ansi.h
infutil.o: /mnt/sda8/git-source/pmon/include/string.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/cdefs.h
infutil.o: /mnt/sda8/git-source/pmon/include/stdlib.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/types.h
infutil.o: /mnt/sda8/git-source/pmon/Targets/Bonito2fdev/compile/Bonito.2fdev.firewall/machine/endian.h
infutil.o: /mnt/sda8/git-source/pmon/sys/sys/endian.h
infutil.o: /mnt/sda8/git-source/pmon/include/errno.h infblock.h inftrees.h
infutil.o: infcodes.h infutil.h
