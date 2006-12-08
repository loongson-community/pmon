IDENT=-DIT8172 -DMIPS -DINET -DCONS_BAUD="B115200" -DAUTOLOAD -DNOSNOOP -DUSE_SUPERIO_UART -DSYSTYPE="\"it8172\"" -DTARGETNAME="\"IT8172\"" -DIDECD
S!=	echo `/bin/pwd`/../../../..
ENDIAN=EL
#	$Id: Makefile,v 1.1.1.1 2006/09/14 01:59:09 root Exp $
#
# Makefile for PMON2000 IT8172
#
# This makefile is constructed from a machine description:
#	config machineid
# Most changes should be made in the machine description
#	/sys/arch/pmonppc/conf/``machineid''
# after which you should do
#	config machineid
# Machine generic makefile changes should be made in
#	/sys/arch/pmonppc/conf/Makefile.pmonppc
# after which config should be rerun for all machines of that type.
#
# N.B.: NO DEPENDENCIES ON FOLLOWING FLAGS ARE VISIBLE TO MAKEFILE
#	IF YOU CHANGE THE DEFINITION OF ANY OF THESE RECOMPILE EVERYTHING
#
.SUFFIXES:	.S .c .o

CROSS_COMPILE 	=mips-elf-

#
# Include the make variables (CC, etc...)
#

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
RANLIB		= $(CROSS_COMPILE)ranlib
SIZE		= $(CROSS_COMPILE)size

OPT?=	-O2

all: pmon

# source tree is located via $S relative to the compilation directory
ifndef S
S:=$(shell cd ../../../..; pwd)
endif

# Defines

TARGET= ${S}/Targets/IT8172
MACHINE=mips
MACHINE_ARCH=mips
COMPILEDIR=${shell pwd}
OBJDIR=${COMPILEDIR}
PMONDIR=${S}


INCLUDES=	-I. -I${S}/include -I./machine -I${S} \
		-I${S}/x86emu/int10/x86emu/include -I${S}/x86emu/int10/x86emu/src/x86emu/x86emu \
		-I${S}/sys/arch/${MACHINE}/include -I${S}/sys \
		-I${TARGET} -I${COMPILEDIR} -nostdinc 
CPPFLAGS=	${INCLUDES} ${IDENT} -D_KERNEL -D__OpenBSD__ -DPMON -D__PMON__\
		-${ENDIAN} -mno-abicalls -mips3 -mcpu=r4000
CWARNFLAGS=	-Wall -Wstrict-prototypes \
		-Wno-uninitialized -Wno-format -Wno-main
CFLAGS=		${DEBUG} ${CWARNFLAGS} ${OPT} -G 0
AFLAGS=		-D_LOCORE -G 0
LFLAGS=		-${ENDIAN} -N -G 0 -T../../conf/ld.script -e start
STRIPFLAGS=	-g -S --strip-debug

HOSTCC?=	${CC}
HOSTED_CPPFLAGS=${CPPFLAGS:S/^-nostdinc$//}
HOSTED_CFLAGS=	${CFLAGS}

include ${S}/lib/libc/Makefile.inc
LIBC=${CLIB}

#include ${S}/lib/libz/Makefile.inc
#LIBZ=${ZLIB}

# compile rules: rules are named ${TYPE}_${SUFFIX}${CONFIG_DEP}
# where TYPE is NORMAL, DRIVER, or PROFILE}; SUFFIX is the file suffix,
# capitalized (e.g. C for a .c file), and CONFIG_DEP is _C if the file
# is marked as config-dependent.

USRLAND_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} -c $<
USRLAND_C_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} ${PARAM} -c $<

NORMAL_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} -c $<
NORMAL_C_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} ${PARAM} -c $<

DRIVER_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} -c $<
DRIVER_C_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} ${PARAM} -c $<

NORMAL_S=	${CC} ${AFLAGS} ${CPPFLAGS} -c $<
IMAGE_S=	${CC} ${AFLAGS} -DPMON_ENTRY=0x80020000 ${CPPFLAGS} -c $<
NORMAL_S_C=	${AS}  ${COPTS} ${PARAM} $< -o $@


OBJS=	wdc.o mainbus.o pcibr.o if.o if_ethersubr.o if_loop.o if_media.o \
	radix.o raw_cb.o raw_usrreq.o route.o rtsock.o if_ether.o in.o \
	in_cksum.o in_pcb.o in_proto.o ip_icmp.o ip_id.o ip_input.o \
	ip_output.o raw_ip.o udp_usrreq.o kern_clock.o kern_misc.o \
	kern_synch.o uipc_domain.o uipc_socket2.o kern_descrip.o \
	kern_proc.o kern_syscall.o sys_generic.o subr_autoconf.o \
	uipc_mbuf.o uipc_syscalls.o init_main.o kern_malloc.o kern_sig.o \
	kern_time.o sys_socket.o uipc_socket.o bus_dma.o boot.o call.o \
	mycmd.o cmdtable.o debuggercmds.o devls.o cmd_go.o hist.o \
	ifaddr.o oload.o load.o memcmds.o memtst.o miscmds.o pcicmds.o \
	pflash.o cmd_env.o stty.o transp.o sym.o shellcmds.o about.o \
	autoconf.o callvec.o cmdparser.o debugger.o exception.o main.o \
	more.o rsa.o sbrk.o env.o termio.o socket.o iso9660fs.o devfs.o \
	diskfs.o ext2fs.o loadfn.o exec.o exec_bin.o exec_elf.o \
	exec_srec.o netio.o bootp.o ifconfig.o ping.o tftplib.o kbd.o \
	vgacon.o generic_poll.o flash.o flashdev.o flash_amd.o \
	flash_int.o flash_sst.o ns16550.o machdep.o mips_machdep.o \
	disassemble.o mips.o cache.o pci_machdep.o tgt_machdep.o \
	localbus.o mii.o pciconf.o pci_map.o pci.o if_fxp.o pciide.o \
	pcidevs.o wd.o ata_wdc.o ata.o cd.o

CFILES=	$S/sys/dev/ic/wdc.c $S/pmon/dev/mainbus.c $S/pmon/dev/pcibr.c \
	$S/sys/net/if.c $S/sys/net/if_ethersubr.c $S/sys/net/if_loop.c \
	$S/sys/net/if_media.c $S/sys/net/radix.c $S/sys/net/raw_cb.c \
	$S/sys/net/raw_usrreq.c $S/sys/net/route.c $S/sys/net/rtsock.c \
	$S/sys/netinet/if_ether.c $S/sys/netinet/in.c \
	$S/sys/netinet/in_cksum.c $S/sys/netinet/in_pcb.c \
	$S/sys/netinet/in_proto.c $S/sys/netinet/ip_icmp.c \
	$S/sys/netinet/ip_id.c $S/sys/netinet/ip_input.c \
	$S/sys/netinet/ip_output.c $S/sys/netinet/raw_ip.c \
	$S/sys/netinet/udp_usrreq.c $S/sys/kern/kern_clock.c \
	$S/sys/kern/kern_misc.c $S/sys/kern/kern_synch.c \
	$S/sys/kern/uipc_domain.c $S/sys/kern/uipc_socket2.c \
	$S/sys/kern/kern_descrip.c $S/sys/kern/kern_proc.c \
	$S/sys/kern/kern_syscall.c $S/sys/kern/sys_generic.c \
	$S/sys/kern/subr_autoconf.c $S/sys/kern/uipc_mbuf.c \
	$S/sys/kern/uipc_syscalls.c $S/sys/kern/init_main.c \
	$S/sys/kern/kern_malloc.c $S/sys/kern/kern_sig.c \
	$S/sys/kern/kern_time.c $S/sys/kern/sys_socket.c \
	$S/sys/kern/uipc_socket.c $S/sys/dev/bus_dma.c \
	$S/pmon/cmds/boot.c $S/pmon/cmds/call.c $S/pmon/cmds/mycmd.c \
	$S/pmon/cmds/cmdtable.c $S/pmon/cmds/debuggercmds.c \
	$S/pmon/cmds/devls.c $S/pmon/cmds/cmd_go.c $S/pmon/cmds/hist.c \
	$S/pmon/cmds/ifaddr.c $S/pmon/cmds/oload.c $S/pmon/cmds/load.c \
	$S/pmon/cmds/memcmds.c $S/pmon/cmds/memtst.c \
	$S/pmon/cmds/miscmds.c $S/pmon/cmds/pcicmds.c \
	$S/pmon/cmds/pflash.c $S/pmon/cmds/cmd_env.c $S/pmon/cmds/stty.c \
	$S/pmon/cmds/transp.c $S/pmon/cmds/sym.c $S/pmon/cmds/shellcmds.c \
	$S/pmon/common/about.c $S/pmon/common/autoconf.c \
	$S/pmon/common/callvec.c $S/pmon/common/cmdparser.c \
	$S/pmon/common/debugger.c $S/pmon/common/exception.c \
	$S/pmon/common/main.c $S/pmon/common/more.c $S/pmon/common/rsa.c \
	$S/pmon/common/sbrk.c $S/pmon/common/env.c $S/pmon/fs/termio.c \
	$S/pmon/fs/socket.c $S/pmon/fs/iso9660fs.c $S/pmon/fs/devfs.c \
	$S/pmon/fs/diskfs.c $S/pmon/fs/ext2fs.c $S/pmon/loaders/loadfn.c \
	$S/pmon/loaders/exec.c $S/pmon/loaders/exec_bin.c \
	$S/pmon/loaders/exec_elf.c $S/pmon/loaders/exec_srec.c \
	$S/pmon/netio/netio.c $S/pmon/netio/bootp.c \
	$S/pmon/netio/ifconfig.c $S/pmon/netio/ping.c \
	$S/pmon/netio/tftplib.c $S/pmon/dev/kbd.c $S/pmon/dev/vgacon.c \
	$S/pmon/dev/generic_poll.c $S/pmon/dev/flash.c \
	$S/pmon/dev/flashdev.c $S/pmon/dev/flash_amd.c \
	$S/pmon/dev/flash_int.c $S/pmon/dev/flash_sst.c \
	$S/pmon/dev/ns16550.c $S/pmon/arch/mips/machdep.c \
	$S/pmon/arch/mips/mips_machdep.c $S/pmon/arch/mips/disassemble.c \
	$S/Targets/IT8172/pci/pci_machdep.c \
	$S/Targets/IT8172/IT8172/tgt_machdep.c \
	$S/Targets/IT8172/dev/localbus.c $S/sys/dev/mii/mii.c \
	$S/sys/dev/pci/pciconf.c $S/sys/dev/pci/pci_map.c \
	$S/sys/dev/pci/pci.c $S/sys/dev/pci/if_fxp.c \
	$S/sys/dev/pci/pciide.c $S/sys/dev/pci/pcidevs.c \
	$S/sys/dev/ata/wd.c $S/sys/dev/ata/ata_wdc.c $S/sys/dev/ata/ata.c \
	$S/sys/dev/ata/cd.c

SFILES=

# load lines for config "xxx" will be emitted as:
# xxx: ${SYSTEM_DEP}
#	${SYSTEM_LD_HEAD}
#	${SYSTEM_LD}
#	${SYSTEM_LD_TAIL}
SYSTEM_OBJ=	starto.o crtbegin.o param.o ioconf.o ${OBJS} ${LIBC} ${LIBZ} \
		crtend.o
SYSTEM_DEP=	Makefile ${SYSTEM_OBJ}
SYSTEM_LD_HEAD=	rm -f $@
SYSTEM_LD=	@echo ${LD} ${LFLAGS} -o $@ ${LIBDIR} "${SYSTEM_OBJ}" vers.o; \
		${LD} ${LFLAGS} -o $@ ${LIBDIR} ${SYSTEM_OBJ} vers.o
SYSTEM_LD_TAIL=	@${SIZE} $@; chmod 755 $@ ; \
		$(OBJCOPY) -O binary $@ $@.bin

DEBUG?=
ifeq (${DEBUG}, "-g")
LFLAGS+=	-X
SYSTEM_LD_TAIL+=; \
		echo cp $@ $@.gdb; rm -f $@.gdb; cp $@ $@.gdb; \
		echo ${STRIP} ${STRIPFLAGS} $@; ${STRIP} ${STRIPFLAGS} $@
else
LFLAGS+=	-S
endif

all: pmon

pmon: ${SYSTEM_DEP} newvers
	${SYSTEM_LD_HEAD}
	${SYSTEM_LD}
	${SYSTEM_LD_TAIL}


param.c: $S/sys/kern/param.c
	rm -f param.c
	cp $S/sys/kern/param.c .

param.o: param.c Makefile
	${NORMAL_C_C}

ioconf.o: ioconf.c
	${NORMAL_C}

crtbegin.c: $S/pmon/arch/mips/crtbegin.c
	rm -f crtbegin.c
	cp $S/pmon/arch/mips/crtbegin.c .

crtbegin.o: crtbegin.c Makefile
	${NORMAL_C_C}

crtend.c: $S/pmon/arch/mips/crtend.c
	rm -f crtend.c
	cp $S/pmon/arch/mips/crtend.c .

crtend.o: crtend.c Makefile
	${NORMAL_C_C}

newvers: ${SYSTEM_DEP} ${SYSTEM_SWAP_DEP}
	sh $S/conf/newvers.sh
	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} -c vers.c

clean::
	rm -f eddep *netbsd netbsd.gdb tags *.[io] [a-z]*.s \
		[Ee]rrs linterrs makelinks genassym genassym.o 

lint:
	@lint -hbxncez -DGENERIC -Dvolatile= ${CPPFLAGS} -UKGDB \
		${CFILES} \
		ioconf.c param.c | \
		grep -v 'static function .* unused'

tags:
	@echo "see $S/kern/Makefile for tags"

links:
	egrep '#if' ${CFILES} | sed -f $S/conf/defines | \
	  sed -e 's/:.*//' -e 's/\.c/.o/' | sort -u > dontlink
	echo ${CFILES} | tr -s ' ' '\12' | sed 's/\.c/.o/' | \
	  sort -u | comm -23 - dontlink | \
	  sed 's,../.*/\(.*.o\),rm -f \1; ln -s ../GENERIC/\1 \1,' > makelinks
	sh makelinks && rm -f dontlink

SRCS=	${TARGET}/IT8172/starto.S \
	param.c ioconf.c ${CFILES} ${SFILES}
depend:: .depend
.depend: ${SRCS} param.c
	export CC
	${MKDEP} ${AFLAGS} ${CPPFLAGS} ${TARGET}/IT8172/starto.S
	${MKDEP} -a ${CFLAGS} ${CPPFLAGS} param.c ioconf.c ${CFILES}
ifneq (${SFILES}, "")
	${MKDEP} -a ${AFLAGS} ${CPPFLAGS} ${SFILES}
endif

# depend on root or device configuration
autoconf.o conf.o: Makefile

# depend on network or filesystem configuration
uipc_proto.o vfs_conf.o: Makefile

zpmon: startz.o
	make -C ../zboot/images
	make -C ../zboot/pmon zpmon

startz.o: ${TARGET}/IT8172/startz.S
	${IMAGE_S}

starto.o: ${TARGET}/IT8172/starto.S
	${NORMAL_S}


wdc.o: $S/sys/dev/ic/wdc.c
	${NORMAL_C}

mainbus.o: $S/pmon/dev/mainbus.c
	${NORMAL_C}

pcibr.o: $S/pmon/dev/pcibr.c
	${NORMAL_C}

if.o: $S/sys/net/if.c
	${NORMAL_C}

if_ethersubr.o: $S/sys/net/if_ethersubr.c
	${NORMAL_C}

if_loop.o: $S/sys/net/if_loop.c
	${NORMAL_C}

if_media.o: $S/sys/net/if_media.c
	${NORMAL_C}

radix.o: $S/sys/net/radix.c
	${NORMAL_C}

raw_cb.o: $S/sys/net/raw_cb.c
	${NORMAL_C}

raw_usrreq.o: $S/sys/net/raw_usrreq.c
	${NORMAL_C}

route.o: $S/sys/net/route.c
	${NORMAL_C}

rtsock.o: $S/sys/net/rtsock.c
	${NORMAL_C}

if_ether.o: $S/sys/netinet/if_ether.c
	${NORMAL_C}

in.o: $S/sys/netinet/in.c
	${NORMAL_C}

in_cksum.o: $S/sys/netinet/in_cksum.c
	${NORMAL_C}

in_pcb.o: $S/sys/netinet/in_pcb.c
	${NORMAL_C}

in_proto.o: $S/sys/netinet/in_proto.c
	${NORMAL_C}

ip_icmp.o: $S/sys/netinet/ip_icmp.c
	${NORMAL_C}

ip_id.o: $S/sys/netinet/ip_id.c
	${NORMAL_C}

ip_input.o: $S/sys/netinet/ip_input.c
	${NORMAL_C}

ip_output.o: $S/sys/netinet/ip_output.c
	${NORMAL_C}

raw_ip.o: $S/sys/netinet/raw_ip.c
	${NORMAL_C}

udp_usrreq.o: $S/sys/netinet/udp_usrreq.c
	${NORMAL_C}

kern_clock.o: $S/sys/kern/kern_clock.c
	${NORMAL_C}

kern_misc.o: $S/sys/kern/kern_misc.c
	${NORMAL_C}

kern_synch.o: $S/sys/kern/kern_synch.c
	${NORMAL_C}

uipc_domain.o: $S/sys/kern/uipc_domain.c
	${NORMAL_C}

uipc_socket2.o: $S/sys/kern/uipc_socket2.c
	${NORMAL_C}

kern_descrip.o: $S/sys/kern/kern_descrip.c
	${NORMAL_C}

kern_proc.o: $S/sys/kern/kern_proc.c
	${NORMAL_C}

kern_syscall.o: $S/sys/kern/kern_syscall.c
	${NORMAL_C}

sys_generic.o: $S/sys/kern/sys_generic.c
	${NORMAL_C}

subr_autoconf.o: $S/sys/kern/subr_autoconf.c
	${NORMAL_C}

uipc_mbuf.o: $S/sys/kern/uipc_mbuf.c
	${NORMAL_C}

uipc_syscalls.o: $S/sys/kern/uipc_syscalls.c
	${NORMAL_C}

init_main.o: $S/sys/kern/init_main.c
	${NORMAL_C}

kern_malloc.o: $S/sys/kern/kern_malloc.c
	${NORMAL_C}

kern_sig.o: $S/sys/kern/kern_sig.c
	${NORMAL_C}

kern_time.o: $S/sys/kern/kern_time.c
	${NORMAL_C}

sys_socket.o: $S/sys/kern/sys_socket.c
	${NORMAL_C}

uipc_socket.o: $S/sys/kern/uipc_socket.c
	${NORMAL_C}

bus_dma.o: $S/sys/dev/bus_dma.c
	${NORMAL_C}

boot.o: $S/pmon/cmds/boot.c
	${NORMAL_C}

call.o: $S/pmon/cmds/call.c
	${NORMAL_C}

mycmd.o: $S/pmon/cmds/mycmd.c
	${NORMAL_C}

cmdtable.o: $S/pmon/cmds/cmdtable.c
	${NORMAL_C}

debuggercmds.o: $S/pmon/cmds/debuggercmds.c
	${NORMAL_C}

devls.o: $S/pmon/cmds/devls.c
	${NORMAL_C}

cmd_go.o: $S/pmon/cmds/cmd_go.c
	${NORMAL_C}

hist.o: $S/pmon/cmds/hist.c
	${NORMAL_C}

ifaddr.o: $S/pmon/cmds/ifaddr.c
	${NORMAL_C}

oload.o: $S/pmon/cmds/oload.c
	${NORMAL_C}

load.o: $S/pmon/cmds/load.c
	${NORMAL_C}

memcmds.o: $S/pmon/cmds/memcmds.c
	${NORMAL_C}

memtst.o: $S/pmon/cmds/memtst.c
	${NORMAL_C}

miscmds.o: $S/pmon/cmds/miscmds.c
	${NORMAL_C}

pcicmds.o: $S/pmon/cmds/pcicmds.c
	${NORMAL_C}

pflash.o: $S/pmon/cmds/pflash.c
	${NORMAL_C}

cmd_env.o: $S/pmon/cmds/cmd_env.c
	${NORMAL_C}

stty.o: $S/pmon/cmds/stty.c
	${NORMAL_C}

transp.o: $S/pmon/cmds/transp.c
	${NORMAL_C}

sym.o: $S/pmon/cmds/sym.c
	${NORMAL_C}

shellcmds.o: $S/pmon/cmds/shellcmds.c
	${NORMAL_C}

about.o: $S/pmon/common/about.c
	${NORMAL_C}

autoconf.o: $S/pmon/common/autoconf.c
	${NORMAL_C}

callvec.o: $S/pmon/common/callvec.c
	${NORMAL_C}

cmdparser.o: $S/pmon/common/cmdparser.c
	${NORMAL_C}

debugger.o: $S/pmon/common/debugger.c
	${NORMAL_C}

exception.o: $S/pmon/common/exception.c
	${NORMAL_C}

main.o: $S/pmon/common/main.c
	${NORMAL_C}

more.o: $S/pmon/common/more.c
	${NORMAL_C}

rsa.o: $S/pmon/common/rsa.c
	${NORMAL_C}

sbrk.o: $S/pmon/common/sbrk.c
	${NORMAL_C}

env.o: $S/pmon/common/env.c
	${NORMAL_C}

termio.o: $S/pmon/fs/termio.c
	${NORMAL_C}

socket.o: $S/pmon/fs/socket.c
	${NORMAL_C}

iso9660fs.o: $S/pmon/fs/iso9660fs.c
	${NORMAL_C}

devfs.o: $S/pmon/fs/devfs.c
	${NORMAL_C}

diskfs.o: $S/pmon/fs/diskfs.c
	${NORMAL_C}

ext2fs.o: $S/pmon/fs/ext2fs.c
	${NORMAL_C}

loadfn.o: $S/pmon/loaders/loadfn.c
	${NORMAL_C}

exec.o: $S/pmon/loaders/exec.c
	${NORMAL_C}

exec_bin.o: $S/pmon/loaders/exec_bin.c
	${NORMAL_C}

exec_elf.o: $S/pmon/loaders/exec_elf.c
	${NORMAL_C}

exec_srec.o: $S/pmon/loaders/exec_srec.c
	${NORMAL_C}

netio.o: $S/pmon/netio/netio.c
	${NORMAL_C}

bootp.o: $S/pmon/netio/bootp.c
	${NORMAL_C}

ifconfig.o: $S/pmon/netio/ifconfig.c
	${NORMAL_C}

ping.o: $S/pmon/netio/ping.c
	${NORMAL_C}

tftplib.o: $S/pmon/netio/tftplib.c
	${NORMAL_C}

kbd.o: $S/pmon/dev/kbd.c
	${NORMAL_C}

vgacon.o: $S/pmon/dev/vgacon.c
	${NORMAL_C}

generic_poll.o: $S/pmon/dev/generic_poll.c
	${NORMAL_C}

flash.o: $S/pmon/dev/flash.c
	${NORMAL_C}

flashdev.o: $S/pmon/dev/flashdev.c
	${NORMAL_C}

flash_amd.o: $S/pmon/dev/flash_amd.c
	${NORMAL_C}

flash_int.o: $S/pmon/dev/flash_int.c
	${NORMAL_C}

flash_sst.o: $S/pmon/dev/flash_sst.c
	${NORMAL_C}

ns16550.o: $S/pmon/dev/ns16550.c
	${NORMAL_C}

machdep.o: $S/pmon/arch/mips/machdep.c
	${NORMAL_C}

mips_machdep.o: $S/pmon/arch/mips/mips_machdep.c
	${NORMAL_C}

disassemble.o: $S/pmon/arch/mips/disassemble.c
	${NORMAL_C}

mips.o: $S/pmon/arch/mips/mips.S
	${NORMAL_S}

cache.o: $S/pmon/arch/mips/cache.S
	${NORMAL_S}

pci_machdep.o: $S/Targets/IT8172/pci/pci_machdep.c
	${NORMAL_C}

tgt_machdep.o: $S/Targets/IT8172/IT8172/tgt_machdep.c
	${NORMAL_C}

localbus.o: $S/Targets/IT8172/dev/localbus.c
	${NORMAL_C}

mii.o: $S/sys/dev/mii/mii.c
	${NORMAL_C}

pciconf.o: $S/sys/dev/pci/pciconf.c
	${NORMAL_C}

pci_map.o: $S/sys/dev/pci/pci_map.c
	${NORMAL_C}

pci.o: $S/sys/dev/pci/pci.c
	${NORMAL_C}

if_fxp.o: $S/sys/dev/pci/if_fxp.c
	${NORMAL_C}

pciide.o: $S/sys/dev/pci/pciide.c
	${NORMAL_C}

pcidevs.o: $S/sys/dev/pci/pcidevs.c
	${NORMAL_C}

wd.o: $S/sys/dev/ata/wd.c
	${NORMAL_C}

ata_wdc.o: $S/sys/dev/ata/ata_wdc.c
	${NORMAL_C}

ata.o: $S/sys/dev/ata/ata.c
	${NORMAL_C}

cd.o: $S/sys/dev/ata/cd.c
	${NORMAL_C}

# DO NOT DELETE

/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/starto.o: ./machine/asm.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/starto.o: ./machine/regdef.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/starto.o: ./machine/regnum.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/starto.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/starto.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/starto.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/starto.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/starto.o: ./machine/pte.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/starto.o: target/it8172.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/starto.o: target/addrspace.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/starto.o: /mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.h

param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h ./machine/types.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h ./machine/cdefs.h
param.o: ./machine/ansi.h ./machine/endian.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h ./machine/signal.h
param.o: ./machine/cpu.h ./machine/psl.h ./machine/intr.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h ./machine/param.h
param.o: ./machine/limits.h /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
param.o: ./machine/stdarg.h ./machine/va-mips.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/callout.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filedesc.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
param.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
ioconf.o: mainbus.h /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h ./machine/types.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h ./machine/cdefs.h
ioconf.o: ./machine/ansi.h ./machine/endian.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h ./machine/signal.h
ioconf.o: ./machine/cpu.h ./machine/psl.h ./machine/intr.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h ./machine/param.h
ioconf.o: ./machine/limits.h /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
ioconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/conf.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/buf.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: ./machine/bus.h ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/atavar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/atareg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdcreg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdc.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdcvar.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: /mnt/sdb4/src/716/pmon-all/include/autoconf.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/mainbus.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/include/autoconf.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/bus.h ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/pcibr.o: /mnt/sdb4/src/716/pmon-all/pmon/dev/pcibrvar.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: bpfilter.h bridge.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: ./machine/cdefs.h ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: ./machine/signal.h ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: ./machine/psl.h ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: ./machine/param.h ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_dl.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_types.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/net/radix.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/net/if.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/net/netisr.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_llc.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_dl.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_types.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_ethersubr.o: bridge.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: bpfilter.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_types.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/net/netisr.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_loop.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_media.h
/mnt/sdb4/src/716/pmon-all/sys/net/if_media.o: /mnt/sdb4/src/716/pmon-all/sys/net/netisr.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/domain.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/net/radix.o: /mnt/sdb4/src/716/pmon-all/sys/net/radix.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/domain.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/domain.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/net/netisr.h
/mnt/sdb4/src/716/pmon-all/sys/net/raw_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/domain.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/net/route.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/domain.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sysctl.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/net/rtsock.o: /mnt/sdb4/src/716/pmon-all/sys/net/raw_cb.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_dl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/igmp_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in.o: ether.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_cksum.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/domain.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_ipsp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/domain.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/net/radix.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_ipsp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/igmp_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/tcp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/tcp_fsm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/tcp_seq.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/tcp_timer.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/tcp_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/tcpip.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/tcp_debug.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/udp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/udp_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/in_proto.o: tcp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sysctl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/icmp_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_id.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/domain.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sysctl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_dl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_ipsp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_input.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_ipsp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/ip_output.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_mroute.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_ipsp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/raw_ip.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/stat.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sysctl.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_pcb.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_ipsp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_var.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/udp.h
/mnt/sdb4/src/716/pmon-all/sys/netinet/udp_usrreq.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/udp_var.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/callout.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resourcevar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_clock.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sysctl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/buf.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/net/netisr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_misc.o: /mnt/sdb4/src/716/pmon-all/sys/linux/io.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/map.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signalvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_synch.o: /mnt/sdb4/src/716/pmon-all/sys/net/netisr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/domain.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_domain.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sysctl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/buf.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket2.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signalvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filedesc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/vnode.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/stat.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resourcevar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/conf.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syscallargs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_descrip.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resourcevar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filedesc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signalvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_proc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signalvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_syscall.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syscallargs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filedesc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signalvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/stat.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/poll.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_generic.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syscallargs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/subr_autoconf.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/map.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/domain.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_mbuf.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filedesc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/buf.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signalvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_syscalls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syscallargs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/callout.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/init_main.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/map.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_malloc.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_kern.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/map.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signalvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_sig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syscallargs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resourcevar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signalvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/kern_time.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syscallargs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/stat.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/kern/sys_socket.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/domain.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/protosw.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socketvar.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/sys/kern/uipc_socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signalvar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_kern.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/bus.h ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/sys/dev/bus_dma.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/include/debugger.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/include/exec.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/boot.o: /mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/call.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: ./machine/bus.h ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: ./flash.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/mycmd.o: /mnt/sdb4/src/716/pmon-all/pmon/cmds/mipsregs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmdtable.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/include/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/debuggercmds.o: /mnt/sdb4/src/716/pmon-all/include/debugger.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/devls.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/include/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./machine/frame.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: /mnt/sdb4/src/716/pmon-all/include/debugger.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_go.o: mod_debugger.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/hist.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/ifaddr.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/include/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: /mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: mod_debugger.h mod_load.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/oload.o: mod_symbols.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/include/exec.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: /mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/load.o: ./flash.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memcmds.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/memtst.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/miscmds.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pcicmds.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: /mnt/sdb4/src/716/pmon-all/include/pflash.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/pflash.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/pflash_tgt.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: /mnt/sdb4/src/716/pmon-all/include/pmonenv.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/cmd_env.o: cmd_env.h cmd_hist.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/stty.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/transp.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/sym.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/include/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/cmds/shellcmds.o: cmd_hist.h cmd_shell.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/common/about.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/conf.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: /mnt/sdb4/src/716/pmon-all/include/autoconf.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/pmon/common/autoconf.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/common/callvec.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/include/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/common/cmdparser.o: cmd_hist.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/include/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./machine/frame.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: /mnt/sdb4/src/716/pmon-all/include/debugger.h
/mnt/sdb4/src/716/pmon-all/pmon/common/debugger.o: mod_symbols.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/include/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./machine/frame.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/include/debugger.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: /mnt/sdb4/src/716/pmon-all/include/ri.h
/mnt/sdb4/src/716/pmon-all/pmon/common/exception.o: mod_debugger.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/exec.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: /mnt/sdb4/src/716/pmon-all/include/file.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: mod_debugger.h mod_symbols.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: sd.h wd.h cmd_hist.h
/mnt/sdb4/src/716/pmon-all/pmon/common/main.o: cmd_more.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/common/more.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/common/rsa.o: mod_debugger.h mod_symbols.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/common/sbrk.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: /mnt/sdb4/src/716/pmon-all/include/pmonenv.h
/mnt/sdb4/src/716/pmon-all/pmon/common/env.o: cmd_more.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/stdarg.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/file.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/termio.o: mod_vgacon.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: /mnt/sdb4/src/716/pmon-all/include/file.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: /mnt/sdb4/src/716/pmon-all/include/stdarg.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/socket.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/stat.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: /mnt/sdb4/src/716/pmon-all/include/file.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/iso9660fs.o: iso9660.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/stat.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/buf.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: /mnt/sdb4/src/716/pmon-all/include/file.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/devfs.o: cd.h sd.h wd.h fd.h ide_cd.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/file.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/diskfs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/diskfs.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/include/file.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.h
/mnt/sdb4/src/716/pmon-all/pmon/fs/ext2fs.o: /mnt/sdb4/src/716/pmon-all/include/diskfs.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: /mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: mod_debugger.h mod_load.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.o: mod_symbols.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec.o: /mnt/sdb4/src/716/pmon-all/include/exec.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/include/exec.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_bin.o: /mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/include/exec.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/pmon/loaders/elf.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: /mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_elf.o: gzip.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/include/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/include/exec.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: /mnt/sdb4/src/716/pmon-all/pmon/loaders/loadfn.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: mod_debugger.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: mod_load.h mod_symbols.h
/mnt/sdb4/src/716/pmon-all/pmon/loaders/exec_srec.o: gzip.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/include/netdb.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/include/stdarg.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/include/file.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/pmon/netio/netio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/netio.o: /mnt/sdb4/src/716/pmon-all/include/url.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_dl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/include/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/setjmp.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/include/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/include/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/pmon/netio/bootparams.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.o: /mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/net/route.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/pmon/netio/bootparams.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ifconfig.o: /mnt/sdb4/src/716/pmon-all/pmon/netio/bootp.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_icmp.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip_var.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/arpa/inet.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/netdb.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/err.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/ping.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/include/arpa/tftp.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/include/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/include/netdb.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/include/unistd.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/include/file.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/pmon/netio/netio.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/include/url.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/netio/tftplib.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/kbd.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/kbd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/kbd.o: ./machine/cdefs.h ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/kbd.o: /mnt/sdb4/src/716/pmon-all/sys/linux/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/kbd.o: /mnt/sdb4/src/716/pmon-all/sys/linux/io.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/kbd.o: /mnt/sdb4/src/716/pmon-all/pmon/dev/kbd.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: /mnt/sdb4/src/716/pmon-all/sys/linux/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/vgacon.o: /mnt/sdb4/src/716/pmon-all/sys/linux/io.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/generic_poll.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: /mnt/sdb4/src/716/pmon-all/include/pflash.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: ./flash.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/pflash_tgt.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: /mnt/sdb4/src/716/pmon-all/include/pflash.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/pflash_tgt.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flashdev.o: ./flash.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: /mnt/sdb4/src/716/pmon-all/include/pflash.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_amd.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/pflash_tgt.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: /mnt/sdb4/src/716/pmon-all/include/pflash.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_int.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/pflash_tgt.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: /mnt/sdb4/src/716/pmon-all/include/pflash.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/flash_sst.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/pflash_tgt.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.o: /mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./machine/frame.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./machine/regnum.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./machine/trap.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/machdep.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/mips_machdep.o: /mnt/sdb4/src/716/pmon-all/include/autoconf.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/include/ctype.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: ./machine/frame.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/pmon/arch/mips/disassemble.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/nppbreg.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/include/it8172.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/include/addrspace.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/include/termio.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/include/queue.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/include/string.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/include/time.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/include/stdlib.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/include/stdarg.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/include/autoconf.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/include/pflash.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/pflash_tgt.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/include/it8172.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/include/addrspace.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/include/rtc.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/pmon/dev/ns16550.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: /mnt/sdb4/src/716/pmon-all/sys/linux/io.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: mod_x86emu.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: mod_x86emu_int10.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: mod_vgacon.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/IT8172/tgt_machdep.o: ./flash.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/conf.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/tty.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/termios.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttydefaults.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/sys/sys/select.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: /mnt/sdb4/src/716/pmon-all/include/autoconf.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/Targets/IT8172/dev/localbus.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_media.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.h
/mnt/sdb4/src/716/pmon-all/sys/dev/mii/mii.o: /mnt/sdb4/src/716/pmon-all/sys/dev/mii/miivar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/include/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/include/pmon_target.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciconf.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci_map.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/bus.h ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pci.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: bpfilter.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/mbuf.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/socket.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/net/if.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_arp.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_dl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_media.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/net/if_types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/in_var.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/ip.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/netinet/if_ether.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/dev/mii/miivar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxpreg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxp.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/if_fxpvar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_kern.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciidereg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciidevar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide_piix_reg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide_cmd_reg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide_acer_reg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide_pdc202xx_reg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/atavar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdcreg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pciide.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdcvar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/include/pmon.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: ./machine/pmon_arch.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/include/progress.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: ./target/pmon_target.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/include/stdio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcivar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/Targets/IT8172/pci/pci_machdep.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcireg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs.o: /mnt/sdb4/src/716/pmon-all/sys/dev/pci/pcidevs_data.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/conf.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/stat.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/buf.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/disklabel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/disklabel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkbad.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/disk.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/lock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/vnode.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: ./machine/bus.h ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/atareg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/atavar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/wdvar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdcreg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/wd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ataio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/cpu.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/stat.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/buf.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/disklabel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/disklabel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkbad.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/bus.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/atareg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/atavar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdcreg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdcvar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata_wdc.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/wdvar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/stat.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslog.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdcreg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/atareg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/ata.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/atavar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/types.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/cdefs.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/ansi.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/endian.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/simplelock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/syslimits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/errno.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/time.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/resource.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ucred.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/uio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/signal.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/cpu.h ./machine/psl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/intr.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/siginfo.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/limits.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/systm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/stdarg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/va-mips.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/kernel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/file.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/fcntl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/unistd.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/queue.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/stat.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioctl.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ttycom.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ioccom.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/filio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/sockio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/buf.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/malloc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/device.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/disklabel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/disklabel.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/dkbad.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/disk.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/lock.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/proc.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/conf.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/scsiio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/vnode.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/vm/vm_param.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/vmparam.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: ./machine/bus.h ./machine/pio.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/atareg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/atavar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ata/wdvar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdcreg.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/dev/ic/wdcvar.h
/mnt/sdb4/src/716/pmon-all/sys/dev/ata/cd.o: /mnt/sdb4/src/716/pmon-all/sys/sys/ataio.h
