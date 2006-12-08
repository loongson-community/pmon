IDENT=-DBONITOEL -DMIPS -DINET -DCONS_BAUD="B115200" -DNOPCINAMES -DAUTOLOAD -DNVRAM_IN_FLASH -DNOSNOOP -DHAVE_NVENV -DHAVE_LOGO -DSYSTYPE="\"Bonito\"" -DTARGETNAME="\"Bonito\"" -DCARD2E -DHAVE_NB_SERIAL
S!=	echo `/bin/pwd`/../../../..
ENDIAN=EL
#	$Id: Makefile.Bonito,v 1.2 2003/12/10 09:23:35 wlin Exp $
#
# Makefile for PMON2000 EV64240
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

TARGET= ${S}/Targets/Bonito2emini
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
		-${ENDIAN} -mno-abicalls -mips3 -mmemcpy -mcpu=r4000
#CWARNFLAGS=	-Werror -Wall -Wmissing-prototypes -Wstrict-prototypes \
		-Wno-uninitialized -Wno-format -Wno-main
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
NORMAL_S_C=	${AS}  ${COPTS} ${PARAM} $< -o $@


OBJS=	wdc.o mainbus.o pcibr.o if.o if_ethersubr.o if_loop.o if_media.o \
	radix.o raw_cb.o raw_usrreq.o route.o rtsock.o if_ether.o in.o \
	in_cksum.o in_pcb.o in_proto.o ip_icmp.o ip_id.o ip_input.o \
	ip_output.o raw_ip.o udp_usrreq.o kern_clock.o kern_misc.o \
	kern_synch.o uipc_domain.o uipc_socket2.o kern_descrip.o \
	kern_proc.o kern_syscall.o sys_generic.o subr_autoconf.o \
	uipc_mbuf.o uipc_syscalls.o init_main.o kern_malloc.o kern_sig.o \
	kern_time.o sys_socket.o uipc_socket.o bus_dma.o boot.o call.o \
	mycmd.o setup.o cmdtable.o debuggercmds.o devls.o cmd_go.o hist.o \
	ifaddr.o oload.o load.o memcmds.o memtst.o miscmds.o pcicmds.o \
	pflash.o cmd_env.o stty.o transp.o sym.o shellcmds.o about.o \
	autoconf.o callvec.o cmdparser.o debugger.o exception.o main.o \
	more.o rsa.o sbrk.o env.o ramfile.o termio.o socket.o devfs.o \
	diskfs.o ext2fs.o loadfn.o exec.o exec_bin.o exec_elf.o \
	exec_srec.o netio.o bootp.o ifconfig.o ping.o tftplib.o debug.o \
	decode.o fpu.o ops.o ops2.o prim_ops.o sys.o generic.o \
	helper_exec.o helper_mem.o xf86int10.o xf86x86emu.o kbd.o \
	vgacon.o generic_poll.o flash.o flashdev.o flash_amd.o \
	flash_int.o flash_sst.o flash_st.o ns16550.o machdep.o \
	mips_machdep.o disassemble.o mips.o cache.o pci_machdep.o \
	tgt_machdep.o localbus.o mii.o mii_physubr.o inphy.o pciconf.o \
	pci_map.o pci.o if_fxp.o rtl8139.o pciide.o pcidevs.o wd.o \
	ata_wdc.o ata.o

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
	$S/pmon/cmds/setup.c $S/pmon/cmds/cmdtable.c \
	$S/pmon/cmds/debuggercmds.c $S/pmon/cmds/devls.c \
	$S/pmon/cmds/cmd_go.c $S/pmon/cmds/hist.c $S/pmon/cmds/ifaddr.c \
	$S/pmon/cmds/oload.c $S/pmon/cmds/load.c $S/pmon/cmds/memcmds.c \
	$S/pmon/cmds/memtst.c $S/pmon/cmds/miscmds.c \
	$S/pmon/cmds/pcicmds.c $S/pmon/cmds/pflash.c \
	$S/pmon/cmds/cmd_env.c $S/pmon/cmds/stty.c $S/pmon/cmds/transp.c \
	$S/pmon/cmds/sym.c $S/pmon/cmds/shellcmds.c \
	$S/pmon/common/about.c $S/pmon/common/autoconf.c \
	$S/pmon/common/callvec.c $S/pmon/common/cmdparser.c \
	$S/pmon/common/debugger.c $S/pmon/common/exception.c \
	$S/pmon/common/main.c $S/pmon/common/more.c $S/pmon/common/rsa.c \
	$S/pmon/common/sbrk.c $S/pmon/common/env.c $S/pmon/fs/ramfile.c \
	$S/pmon/fs/termio.c $S/pmon/fs/socket.c $S/pmon/fs/devfs.c \
	$S/pmon/fs/diskfs.c $S/pmon/fs/ext2fs.c $S/pmon/loaders/loadfn.c \
	$S/pmon/loaders/exec.c $S/pmon/loaders/exec_bin.c \
	$S/pmon/loaders/exec_elf.c $S/pmon/loaders/exec_srec.c \
	$S/pmon/netio/netio.c $S/pmon/netio/bootp.c \
	$S/pmon/netio/ifconfig.c $S/pmon/netio/ping.c \
	$S/pmon/netio/tftplib.c $S/x86emu/int10/x86emu/src/x86emu/debug.c \
	$S/x86emu/int10/x86emu/src/x86emu/decode.c \
	$S/x86emu/int10/x86emu/src/x86emu/fpu.c \
	$S/x86emu/int10/x86emu/src/x86emu/ops.c \
	$S/x86emu/int10/x86emu/src/x86emu/ops2.c \
	$S/x86emu/int10/x86emu/src/x86emu/prim_ops.c \
	$S/x86emu/int10/x86emu/src/x86emu/sys.c $S/x86emu/int10/generic.c \
	$S/x86emu/int10/helper_exec.c $S/x86emu/int10/helper_mem.c \
	$S/x86emu/int10/xf86int10.c $S/x86emu/int10/xf86x86emu.c \
	$S/pmon/dev/kbd.c $S/pmon/dev/vgacon.c $S/pmon/dev/generic_poll.c \
	$S/pmon/dev/flash.c $S/pmon/dev/flashdev.c \
	$S/pmon/dev/flash_amd.c $S/pmon/dev/flash_int.c \
	$S/pmon/dev/flash_sst.c $S/pmon/dev/flash_st.c \
	$S/pmon/dev/ns16550.c $S/pmon/arch/mips/machdep.c \
	$S/pmon/arch/mips/mips_machdep.c $S/pmon/arch/mips/disassemble.c \
	$S/Targets/Bonito2emini/pci/pci_machdep.c \
	$S/Targets/Bonito2emini/Bonito/tgt_machdep.c \
	$S/Targets/Bonito2emini/dev/localbus.c $S/sys/dev/mii/mii.c \
	$S/sys/dev/mii/mii_physubr.c $S/sys/dev/mii/inphy.c \
	$S/sys/dev/pci/pciconf.c $S/sys/dev/pci/pci_map.c \
	$S/sys/dev/pci/pci.c $S/sys/dev/pci/if_fxp.c \
	$S/sys/dev/pci/rtl8139.c $S/sys/dev/pci/pciide.c \
	$S/sys/dev/pci/pcidevs.c $S/sys/dev/ata/wd.c \
	$S/sys/dev/ata/ata_wdc.c $S/sys/dev/ata/ata.c

SFILES=

# load lines for config "xxx" will be emitted as:
# xxx: ${SYSTEM_DEP}
#	${SYSTEM_LD_HEAD}
#	${SYSTEM_LD}
#	${SYSTEM_LD_TAIL}
SYSTEM_OBJ=	start.o crtbegin.o param.o ioconf.o ri.o ${OBJS} ${LIBC} \
		crtend.o
SYSTEM_DEP=	Makefile ${SYSTEM_OBJ}
SYSTEM_LD_HEAD=	rm -f $@
SYSTEM_LD=	@echo ${LD} ${LFLAGS} -o $@ ${LIBDIR} '$${SYSTEM_OBJ}' vers.o; \
		${LD} ${LFLAGS} -o $@ ${LIBDIR} ${SYSTEM_OBJ} vers.o
SYSTEM_LD_TAIL=	@${SIZE} $@; chmod 755 $@ ; \
		${OBJCOPY} -O binary $@ $@.bin

DEBUG?=-g
ifneq (${DEBUG}, "")
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
ri.o:ri.c Makefile
	${NORMAL_C_C}
ri.c: $S/pmon/arch/mips/ri.c
	rm -f ri.c
	cp $S/pmon/arch/mips/ri.c .
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

SRCS=	${TARGET}/Bonito/start.S \
	param.c ioconf.c ri.c ${CFILES} ${SFILES}
depend:: .depend
.depend: ${SRCS} param.c
	${MKDEP} ${AFLAGS} ${CPPFLAGS} ${TARGET}/Bonito/start.S
	${MKDEP} -a ${CFLAGS} ${CPPFLAGS} param.c ioconf.c ${CFILES}
ifneq (${SFILES}, "")
	${MKDEP} -a ${AFLAGS} ${CPPFLAGS} ${SFILES}
endif

# depend on root or device configuration
autoconf.o conf.o: Makefile

# depend on network or filesystem configuration
uipc_proto.o vfs_conf.o: Makefile

start.o: ${TARGET}/Bonito/start.S Makefile
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

setup.o: $S/pmon/cmds/setup.c
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

ramfile.o: $S/pmon/fs/ramfile.c
	${NORMAL_C}

termio.o: $S/pmon/fs/termio.c
	${NORMAL_C}

socket.o: $S/pmon/fs/socket.c
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

debug.o: $S/x86emu/int10/x86emu/src/x86emu/debug.c
	${NORMAL_C}

decode.o: $S/x86emu/int10/x86emu/src/x86emu/decode.c
	${NORMAL_C}

fpu.o: $S/x86emu/int10/x86emu/src/x86emu/fpu.c
	${NORMAL_C}

ops.o: $S/x86emu/int10/x86emu/src/x86emu/ops.c
	${NORMAL_C}

ops2.o: $S/x86emu/int10/x86emu/src/x86emu/ops2.c
	${NORMAL_C}

prim_ops.o: $S/x86emu/int10/x86emu/src/x86emu/prim_ops.c
	${NORMAL_C}

sys.o: $S/x86emu/int10/x86emu/src/x86emu/sys.c
	${NORMAL_C}

generic.o: $S/x86emu/int10/generic.c
	${NORMAL_C}

helper_exec.o: $S/x86emu/int10/helper_exec.c
	${NORMAL_C}

helper_mem.o: $S/x86emu/int10/helper_mem.c
	${NORMAL_C}

xf86int10.o: $S/x86emu/int10/xf86int10.c
	${NORMAL_C}

xf86x86emu.o: $S/x86emu/int10/xf86x86emu.c
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

flash_st.o: $S/pmon/dev/flash_st.c
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

pci_machdep.o: $S/Targets/Bonito2emini/pci/pci_machdep.c
	${NORMAL_C}

tgt_machdep.o: $S/Targets/Bonito2emini/Bonito/tgt_machdep.c
	${NORMAL_C}

localbus.o: $S/Targets/Bonito2emini/dev/localbus.c
	${NORMAL_C}

mii.o: $S/sys/dev/mii/mii.c
	${NORMAL_C}

mii_physubr.o: $S/sys/dev/mii/mii_physubr.c
	${NORMAL_C}

inphy.o: $S/sys/dev/mii/inphy.c
	${NORMAL_C}

pciconf.o: $S/sys/dev/pci/pciconf.c
	${NORMAL_C}

pci_map.o: $S/sys/dev/pci/pci_map.c
	${NORMAL_C}

pci.o: $S/sys/dev/pci/pci.c
	${NORMAL_C}

if_fxp.o: $S/sys/dev/pci/if_fxp.c
	${NORMAL_C}

rtl8139.o: $S/sys/dev/pci/rtl8139.c
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

# DO NOT DELETE

/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: ./machine/asm.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: ./machine/regdef.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: ./machine/regnum.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: ./machine/cpu.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: ./machine/psl.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: ./machine/cpu.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: ./machine/intr.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: ./machine/pte.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: /home/knoppix/pmon-all/pmon/dev/ns16550.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: target/i82371eb.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: target/prid.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: target/sbd.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: target/bonito.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: target/i8254.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: target/pc97307.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: target/isapnpreg.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: machine/newtest/mydebug.S
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: machine/newtest/writeone-testone8.S
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: machine/newtest/writesome+-testsome--8.S
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: machine/newtest/writesome-testsome-rom8.S
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: machine/newtest/writeall-testall8.S
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: machine/newtest/writesome-testsome-ram8.S
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: machine/newtest/writeall-testall-ram8.S
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/start.o: machine/newtest/copytest.S

param.o: /home/knoppix/pmon-all/sys/sys/param.h
param.o: /home/knoppix/pmon-all/sys/sys/types.h ./machine/types.h
param.o: /home/knoppix/pmon-all/sys/sys/cdefs.h ./machine/cdefs.h
param.o: ./machine/ansi.h ./machine/endian.h
param.o: /home/knoppix/pmon-all/sys/sys/endian.h
param.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
param.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
param.o: /home/knoppix/pmon-all/sys/sys/errno.h
param.o: /home/knoppix/pmon-all/sys/sys/time.h
param.o: /home/knoppix/pmon-all/sys/sys/resource.h
param.o: /home/knoppix/pmon-all/sys/sys/ucred.h
param.o: /home/knoppix/pmon-all/sys/sys/uio.h
param.o: /home/knoppix/pmon-all/sys/sys/signal.h ./machine/signal.h
param.o: ./machine/cpu.h ./machine/psl.h ./machine/intr.h
param.o: /home/knoppix/pmon-all/sys/sys/siginfo.h ./machine/param.h
param.o: ./machine/limits.h /home/knoppix/pmon-all/sys/sys/systm.h
param.o: ./machine/stdarg.h ./machine/va-mips.h
param.o: /home/knoppix/pmon-all/sys/sys/callout.h
param.o: /home/knoppix/pmon-all/sys/sys/filedesc.h
param.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
param.o: /home/knoppix/pmon-all/sys/sys/malloc.h
ioconf.o: mainbus.h /home/knoppix/pmon-all/sys/sys/param.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/types.h ./machine/types.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/cdefs.h ./machine/cdefs.h
ioconf.o: ./machine/ansi.h ./machine/endian.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/endian.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/errno.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/time.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/resource.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/ucred.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/uio.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/signal.h ./machine/signal.h
ioconf.o: ./machine/cpu.h ./machine/psl.h ./machine/intr.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/siginfo.h ./machine/param.h
ioconf.o: ./machine/limits.h /home/knoppix/pmon-all/sys/sys/device.h
ioconf.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/conf.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/buf.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: ./machine/vmparam.h ./machine/bus.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: ./machine/pio.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/dev/ata/atavar.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/dev/ata/atareg.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/dev/ic/wdcreg.h
/home/knoppix/pmon-all/sys/dev/ic/wdc.o: /home/knoppix/pmon-all/sys/dev/ic/wdcvar.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: ./machine/param.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: ./machine/limits.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: /home/knoppix/pmon-all/include/autoconf.h
/home/knoppix/pmon-all/pmon/dev/mainbus.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./machine/stdarg.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./machine/va-mips.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./machine/vmparam.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/include/autoconf.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/sys/dev/pci/pcidevs.h
/home/knoppix/pmon-all/pmon/dev/pcibr.o: /home/knoppix/pmon-all/pmon/dev/pcibrvar.h
/home/knoppix/pmon-all/sys/net/if.o: bpfilter.h bridge.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/net/if.o: ./machine/types.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/net/if.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/net/if.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/net/if.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/net/if.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/net/if.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/net/if.o: ./machine/stdarg.h ./machine/va-mips.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/net/if_dl.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/net/if_types.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/net/radix.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/net/if.o: /home/knoppix/pmon-all/sys/netinet/if_ether.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/types.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/param.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/net/netisr.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/net/if_llc.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/net/if_dl.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/net/if_types.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: /home/knoppix/pmon-all/sys/netinet/if_ether.h
/home/knoppix/pmon-all/sys/net/if_ethersubr.o: bridge.h
/home/knoppix/pmon-all/sys/net/if_loop.o: bpfilter.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/net/if_loop.o: ./machine/types.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/net/if_loop.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/net/if_loop.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/net/if_loop.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/net/if_loop.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/net/if_loop.o: ./machine/param.h
/home/knoppix/pmon-all/sys/net/if_loop.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/net/if_loop.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/net/if_loop.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/net/if_types.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/net/netisr.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/net/if_loop.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/net/if_media.o: ./machine/types.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/net/if_media.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/net/if_media.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/net/if_media.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/net/if_media.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/net/if_media.o: ./machine/param.h
/home/knoppix/pmon-all/sys/net/if_media.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/net/if_media.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/net/if_media.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/net/if_media.h
/home/knoppix/pmon-all/sys/net/if_media.o: /home/knoppix/pmon-all/sys/net/netisr.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/net/radix.o: ./machine/types.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/net/radix.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/net/radix.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/net/radix.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/net/radix.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/net/radix.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/net/radix.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/net/radix.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/domain.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/net/radix.o: /home/knoppix/pmon-all/sys/net/radix.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: ./machine/types.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/domain.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/net/raw_cb.h
/home/knoppix/pmon-all/sys/net/raw_cb.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: ./machine/types.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: ./machine/param.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/domain.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/net/netisr.h
/home/knoppix/pmon-all/sys/net/raw_usrreq.o: /home/knoppix/pmon-all/sys/net/raw_cb.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/net/route.o: ./machine/types.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/net/route.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/net/route.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/net/route.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/net/route.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/net/route.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/net/route.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/net/route.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/domain.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/net/raw_cb.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/net/route.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/net/rtsock.o: ./machine/types.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/net/rtsock.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/net/rtsock.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/net/rtsock.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/net/rtsock.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/net/rtsock.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/net/rtsock.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/net/rtsock.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/domain.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/net/rtsock.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/sysctl.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/net/rtsock.o: /home/knoppix/pmon-all/sys/net/raw_cb.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/types.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/param.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/net/if_dl.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/sys/netinet/if_ether.o: /home/knoppix/pmon-all/sys/netinet/if_ether.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/netinet/in.o: ./machine/types.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/netinet/in.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/netinet/in.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/netinet/in.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/netinet/in.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/netinet/in.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/netinet/in.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/netinet/in.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/netinet/if_ether.h
/home/knoppix/pmon-all/sys/netinet/in.o: /home/knoppix/pmon-all/sys/netinet/igmp_var.h
/home/knoppix/pmon-all/sys/netinet/in.o: ether.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/types.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/param.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/netinet/in_cksum.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: ./machine/types.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: ./machine/param.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/domain.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/netinet/in_pcb.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/netinet/ip_ipsp.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/netinet/in_pcb.o: /home/knoppix/pmon-all/sys/netinet/ip_var.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: ./machine/types.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: ./machine/param.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/domain.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/net/radix.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/ip_var.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/ip_icmp.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/in_pcb.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/ip_ipsp.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/igmp_var.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/tcp.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/tcp_fsm.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/tcp_seq.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/tcp_timer.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/tcp_var.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/tcpip.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/tcp_debug.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/udp.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: /home/knoppix/pmon-all/sys/netinet/udp_var.h
/home/knoppix/pmon-all/sys/netinet/in_proto.o: tcp.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/types.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/param.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/sysctl.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/netinet/ip_icmp.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/netinet/ip_var.h
/home/knoppix/pmon-all/sys/netinet/ip_icmp.o: /home/knoppix/pmon-all/sys/netinet/icmp_var.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: ./machine/types.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: ./machine/param.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/netinet/ip_id.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/types.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/param.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/domain.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/sysctl.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/net/if_dl.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/netinet/if_ether.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/netinet/in_pcb.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/netinet/ip_ipsp.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/netinet/ip_var.h
/home/knoppix/pmon-all/sys/netinet/ip_input.o: /home/knoppix/pmon-all/sys/netinet/ip_icmp.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/types.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/param.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/netinet/in_pcb.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/netinet/ip_ipsp.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/netinet/ip_output.o: /home/knoppix/pmon-all/sys/netinet/ip_var.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: ./machine/types.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: ./machine/param.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/netinet/ip_mroute.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/netinet/ip_var.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/netinet/in_pcb.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/netinet/ip_ipsp.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/netinet/raw_ip.o: /home/knoppix/pmon-all/sys/netinet/ip_icmp.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/types.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/param.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/stat.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/sysctl.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/netinet/in_pcb.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/netinet/ip_ipsp.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/netinet/ip_var.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/netinet/ip_icmp.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/netinet/udp.h
/home/knoppix/pmon-all/sys/netinet/udp_usrreq.o: /home/knoppix/pmon-all/sys/netinet/udp_var.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/callout.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/sys/resourcevar.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/kern/kern_clock.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/sysctl.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/sys/buf.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/net/if_types.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/net/netisr.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./target/pmon_target.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: ./target/bonito.h
/home/knoppix/pmon-all/sys/kern/kern_misc.o: /home/knoppix/pmon-all/sys/linux/io.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/map.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/sys/signalvar.h
/home/knoppix/pmon-all/sys/kern/kern_synch.o: /home/knoppix/pmon-all/sys/net/netisr.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/domain.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/kern/uipc_domain.o: /home/knoppix/pmon-all/sys/sys/sysctl.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/buf.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/kern/uipc_socket2.o: /home/knoppix/pmon-all/sys/sys/signalvar.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/filedesc.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/vnode.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/stat.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/resourcevar.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/conf.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/sys/syscallargs.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/kern/kern_descrip.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/resourcevar.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/filedesc.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/signalvar.h
/home/knoppix/pmon-all/sys/kern/kern_proc.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/signalvar.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/kern/kern_syscall.o: /home/knoppix/pmon-all/sys/sys/syscallargs.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/filedesc.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/signalvar.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/stat.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/poll.h
/home/knoppix/pmon-all/sys/kern/sys_generic.o: /home/knoppix/pmon-all/sys/sys/syscallargs.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/subr_autoconf.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/map.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/domain.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/kern/uipc_mbuf.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/filedesc.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/buf.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/signalvar.h
/home/knoppix/pmon-all/sys/kern/uipc_syscalls.o: /home/knoppix/pmon-all/sys/sys/syscallargs.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/callout.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/net/if_types.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./target/pmon_target.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./target/bonito.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/kern/init_main.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/kern/init_main.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/map.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/kern/kern_malloc.o: /home/knoppix/pmon-all/sys/vm/vm_kern.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/map.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/signalvar.h
/home/knoppix/pmon-all/sys/kern/kern_sig.o: /home/knoppix/pmon-all/sys/sys/syscallargs.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/resourcevar.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/signalvar.h
/home/knoppix/pmon-all/sys/kern/kern_time.o: /home/knoppix/pmon-all/sys/sys/syscallargs.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/sys/stat.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/kern/sys_socket.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/types.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/param.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/domain.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/protosw.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/socketvar.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/sys/kern/uipc_socket.o: /home/knoppix/pmon-all/sys/sys/signalvar.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/vm/vm_kern.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./target/pmon_target.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./target/bonito.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/sys/dev/bus_dma.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: ./machine/types.h ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/include/debugger.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/include/exec.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/cmds/boot.o: /home/knoppix/pmon-all/pmon/loaders/loadfn.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/call.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/call.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/call.o: ./machine/types.h ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/call.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/pmon/cmds/call.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/call.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/call.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/call.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/call.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/dev/pci/pcidevs.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: ./flash.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/pmon/cmds/mipsregs.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/errno.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/include/pflash.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/pmon/cmds/mycmd.o: /home/knoppix/pmon-all/sys/net/radix.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/sys/dev/pci/pcidevs.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./flash.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/time.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/linux/zlib.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/linux/zconf.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/include/stdarg.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./machine/stdarg.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: ./machine/va-mips.h
/home/knoppix/pmon-all/pmon/cmds/setup.o: /home/knoppix/pmon-all/pmon/cmds/setup.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/cmdtable.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/include/signal.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./machine/signal.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./machine/cpu.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./machine/psl.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/debuggercmds.o: /home/knoppix/pmon-all/include/debugger.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/devls.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/include/signal.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: ./machine/frame.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: /home/knoppix/pmon-all/include/debugger.h
/home/knoppix/pmon-all/pmon/cmds/cmd_go.o: mod_debugger.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: ./machine/types.h ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/hist.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: ./machine/intr.h ./machine/bus.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/pmon/cmds/ifaddr.o: /home/knoppix/pmon-all/sys/dev/pci/pcidevs.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: ./machine/param.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: ./machine/limits.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/include/signal.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: /home/knoppix/pmon-all/pmon/loaders/loadfn.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: mod_debugger.h mod_load.h
/home/knoppix/pmon-all/pmon/cmds/oload.o: mod_symbols.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/load.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/load.o: ./machine/types.h ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/cmds/load.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/cmds/load.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/cmds/load.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/load.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/load.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/load.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/include/exec.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/cmds/load.o: /home/knoppix/pmon-all/pmon/loaders/loadfn.h
/home/knoppix/pmon-all/pmon/cmds/load.o: ./flash.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/memcmds.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/memtst.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/miscmds.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: ./machine/intr.h ./machine/bus.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/pmon/cmds/pcicmds.o: /home/knoppix/pmon-all/sys/dev/pci/pcidevs.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: /home/knoppix/pmon-all/include/pflash.h
/home/knoppix/pmon-all/pmon/cmds/pflash.o: /home/knoppix/pmon-all/Targets/Bonito2emini/dev/pflash_tgt.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: /home/knoppix/pmon-all/include/pmonenv.h
/home/knoppix/pmon-all/pmon/cmds/cmd_env.o: cmd_env.h cmd_hist.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: ./machine/types.h ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/stty.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/transp.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: ./machine/types.h ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/sym.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/include/signal.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./machine/signal.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: ./target/bonito.h cmd_hist.h
/home/knoppix/pmon-all/pmon/cmds/shellcmds.o: cmd_shell.h
/home/knoppix/pmon-all/pmon/common/about.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/common/about.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/common/about.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/common/about.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/common/about.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/common/about.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/common/about.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/common/about.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/about.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/common/about.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/common/about.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/common/about.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/common/about.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/signal.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/cpu.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/psl.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/param.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/limits.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/stdarg.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/va-mips.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/conf.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: /home/knoppix/pmon-all/include/autoconf.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/bus.h
/home/knoppix/pmon-all/pmon/common/autoconf.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/common/callvec.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/common/callvec.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/common/callvec.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/common/callvec.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/common/callvec.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/common/callvec.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/callvec.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/common/callvec.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/common/callvec.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/common/callvec.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/common/callvec.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/common/callvec.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/common/callvec.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/common/callvec.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/common/callvec.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/common/callvec.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/common/callvec.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/common/callvec.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/include/signal.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./machine/signal.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./machine/cpu.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./machine/psl.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/common/cmdparser.o: ./target/bonito.h cmd_hist.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/include/signal.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./machine/signal.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./machine/cpu.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./machine/psl.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./machine/frame.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/common/debugger.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/common/debugger.o: /home/knoppix/pmon-all/include/debugger.h
/home/knoppix/pmon-all/pmon/common/debugger.o: mod_symbols.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/include/signal.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./machine/signal.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./machine/cpu.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./machine/psl.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./machine/frame.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/common/exception.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/include/debugger.h
/home/knoppix/pmon-all/pmon/common/exception.o: /home/knoppix/pmon-all/include/ri.h
/home/knoppix/pmon-all/pmon/common/exception.o: mod_debugger.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/common/main.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/common/main.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/signal.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/common/main.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/common/main.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/common/main.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/common/main.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/common/main.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/common/main.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/common/main.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/common/main.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/exec.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/common/main.o: /home/knoppix/pmon-all/include/file.h
/home/knoppix/pmon-all/pmon/common/main.o: mod_debugger.h mod_symbols.h sd.h
/home/knoppix/pmon-all/pmon/common/main.o: wd.h cmd_hist.h cmd_more.h
/home/knoppix/pmon-all/pmon/common/more.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/common/more.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/common/more.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/common/more.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/common/more.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/common/more.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/common/more.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/common/more.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/common/more.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/common/more.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/more.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/common/more.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/common/more.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/common/more.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/common/more.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/common/rsa.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/common/rsa.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/common/rsa.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/common/rsa.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/common/rsa.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/common/rsa.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/common/rsa.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/common/rsa.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/common/rsa.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/rsa.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/common/rsa.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/common/rsa.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/common/rsa.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/common/rsa.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/common/rsa.o: ./target/bonito.h mod_debugger.h
/home/knoppix/pmon-all/pmon/common/rsa.o: mod_symbols.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/common/sbrk.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/common/env.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/common/env.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/common/env.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/common/env.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/common/env.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/common/env.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/common/env.o: /home/knoppix/pmon-all/include/pmonenv.h
/home/knoppix/pmon-all/pmon/common/env.o: cmd_more.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/file.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/ramfile.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/errno.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: logfile.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/fs/ramfile.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/stdarg.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./machine/stdarg.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./machine/va-mips.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/errno.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/file.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/fs/termio.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/fs/termio.o: ./target/bonito.h mod_vgacon.h
/home/knoppix/pmon-all/pmon/fs/socket.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/fs/socket.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/fs/socket.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/fs/socket.o: /home/knoppix/pmon-all/include/file.h
/home/knoppix/pmon-all/pmon/fs/socket.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/fs/socket.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/fs/socket.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/fs/socket.o: ./machine/types.h ./machine/endian.h
/home/knoppix/pmon-all/pmon/fs/socket.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/fs/socket.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/fs/socket.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/fs/socket.o: /home/knoppix/pmon-all/include/stdarg.h
/home/knoppix/pmon-all/pmon/fs/socket.o: ./machine/stdarg.h
/home/knoppix/pmon-all/pmon/fs/socket.o: ./machine/va-mips.h
/home/knoppix/pmon-all/pmon/fs/socket.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/stat.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/buf.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: /home/knoppix/pmon-all/include/file.h
/home/knoppix/pmon-all/pmon/fs/devfs.o: cd.h sd.h wd.h fd.h ide_cd.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/file.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/diskfs.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: ./machine/types.h ./machine/endian.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/errno.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/fs/diskfs.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: ./machine/types.h ./machine/endian.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/include/file.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/pmon/fs/ext2fs.h
/home/knoppix/pmon-all/pmon/fs/ext2fs.o: /home/knoppix/pmon-all/include/diskfs.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: /home/knoppix/pmon-all/pmon/loaders/loadfn.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: mod_debugger.h mod_load.h
/home/knoppix/pmon-all/pmon/loaders/loadfn.o: mod_symbols.h
/home/knoppix/pmon-all/pmon/loaders/exec.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/loaders/exec.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/loaders/exec.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/loaders/exec.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/loaders/exec.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/loaders/exec.o: /home/knoppix/pmon-all/include/exec.h
/home/knoppix/pmon-all/pmon/loaders/exec.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/include/exec.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/loaders/exec_bin.o: /home/knoppix/pmon-all/pmon/loaders/loadfn.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./machine/signal.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./machine/cpu.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./machine/psl.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./machine/param.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./machine/limits.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/include/exec.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/pmon/loaders/elf.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: /home/knoppix/pmon-all/pmon/loaders/loadfn.h
/home/knoppix/pmon-all/pmon/loaders/exec_elf.o: gzip.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/include/fcntl.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/include/exec.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: /home/knoppix/pmon-all/pmon/loaders/loadfn.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: mod_debugger.h mod_load.h
/home/knoppix/pmon-all/pmon/loaders/exec_srec.o: mod_symbols.h gzip.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/netio/netio.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/netio/netio.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/netio/netio.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/netio/netio.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/netio/netio.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/netio/netio.o: ./machine/param.h
/home/knoppix/pmon-all/pmon/netio/netio.o: ./machine/limits.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/include/netdb.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/include/stdarg.h
/home/knoppix/pmon-all/pmon/netio/netio.o: ./machine/stdarg.h
/home/knoppix/pmon-all/pmon/netio/netio.o: ./machine/va-mips.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/include/file.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/pmon/netio/netio.h
/home/knoppix/pmon-all/pmon/netio/netio.o: /home/knoppix/pmon-all/include/url.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: ./machine/param.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: ./machine/limits.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/sys/net/if_dl.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/include/setjmp.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: ./machine/setjmp.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/include/errno.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/include/signal.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/pmon/netio/bootparams.h
/home/knoppix/pmon-all/pmon/netio/bootp.o: /home/knoppix/pmon-all/pmon/netio/bootp.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/net/route.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/pmon/netio/bootparams.h
/home/knoppix/pmon-all/pmon/netio/ifconfig.o: /home/knoppix/pmon-all/pmon/netio/bootp.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/netio/ping.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/netio/ping.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/netio/ping.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/netio/ping.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/pmon/netio/ping.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/netio/ping.o: ./machine/param.h
/home/knoppix/pmon-all/pmon/netio/ping.o: ./machine/limits.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/netinet/ip_icmp.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/sys/netinet/ip_var.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/arpa/inet.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/netdb.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/signal.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/err.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/errno.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/netio/ping.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/netio/ping.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/netio/ping.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/netio/ping.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./machine/signal.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./machine/param.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./machine/limits.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/include/arpa/tftp.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/include/errno.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/include/netdb.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/include/unistd.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/include/file.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/pmon/netio/netio.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/include/url.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/netio/tftplib.o: ./target/bonito.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/x86emui.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/debug.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/decode.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_asm.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/fpu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/fpu_regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: /home/knoppix/pmon-all/include/stdarg.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: ./machine/stdarg.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/debug.o: ./machine/va-mips.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/x86emui.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/debug.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/decode.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_asm.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/fpu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/fpu_regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/decode.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/x86emui.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/debug.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/decode.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_asm.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/fpu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/fpu_regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/fpu.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/x86emui.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/debug.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/decode.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_asm.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/fpu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/fpu_regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/x86emui.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/debug.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/decode.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_asm.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/fpu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/fpu_regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/ops2.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/x86emui.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/debug.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/decode.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_asm.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/fpu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/fpu_regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/prim_ops.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/debug.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_ops.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/x86emu/prim_asm.h
/home/knoppix/pmon-all/x86emu/int10/x86emu/src/x86emu/sys.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: ./machine/bus.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: ./machine/pio.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: ./target/pmon_target.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: ./target/bonito.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/x86emu/int10/xf86int10.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/x86emu/int10/xf86x86emu.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/generic.o: /home/knoppix/pmon-all/sys/linux/io.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/x86emu/int10/xf86x86emu.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/x86emu/int10/xf86int10.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: ./machine/bus.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: ./machine/pio.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: ./target/pmon_target.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: ./target/bonito.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/x86emu/int10/helper_exec.o: /home/knoppix/pmon-all/sys/linux/io.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/x86emu/int10/xf86x86emu.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/x86emu/int10/xf86int10.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: ./machine/bus.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: ./machine/pio.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: ./target/pmon_target.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: ./target/bonito.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/x86emu/int10/helper_mem.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/x86emu/int10/xf86int10.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: ./machine/bus.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: ./machine/pio.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: ./target/pmon_target.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: ./target/bonito.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/x86emu/int10/xf86x86emu.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/xf86int10.o: /home/knoppix/pmon-all/sys/linux/io.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/types.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: ./machine/types.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: ./machine/cdefs.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: ./machine/ansi.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: ./machine/endian.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/x86emu/int10/x86emu/include/x86emu/regs.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/x86emu/int10/xf86.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/x86emu/int10/xf86int10.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: ./machine/bus.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: ./machine/pio.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: ./target/pmon_target.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: ./target/bonito.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/x86emu/int10/xf86x86emu.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: /home/knoppix/pmon-all/include/stdarg.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: ./machine/stdarg.h
/home/knoppix/pmon-all/x86emu/int10/xf86x86emu.o: ./machine/va-mips.h
/home/knoppix/pmon-all/pmon/dev/kbd.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/dev/kbd.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/kbd.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/kbd.o: /home/knoppix/pmon-all/sys/linux/types.h
/home/knoppix/pmon-all/pmon/dev/kbd.o: /home/knoppix/pmon-all/sys/linux/io.h
/home/knoppix/pmon-all/pmon/dev/kbd.o: /home/knoppix/pmon-all/pmon/dev/kbd.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: /home/knoppix/pmon-all/sys/linux/types.h
/home/knoppix/pmon-all/pmon/dev/vgacon.o: /home/knoppix/pmon-all/sys/linux/io.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: ./machine/cpu.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: ./machine/psl.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/dev/generic_poll.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/dev/flash.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/dev/flash.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/flash.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/flash.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/dev/flash.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/dev/flash.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/dev/flash.o: ./machine/types.h ./machine/endian.h
/home/knoppix/pmon-all/pmon/dev/flash.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/dev/flash.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/dev/flash.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/dev/flash.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/dev/flash.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/dev/flash.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/dev/flash.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/dev/flash.o: /home/knoppix/pmon-all/include/pflash.h
/home/knoppix/pmon-all/pmon/dev/flash.o: ./flash.h
/home/knoppix/pmon-all/pmon/dev/flash.o: /home/knoppix/pmon-all/Targets/Bonito2emini/dev/pflash_tgt.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: /home/knoppix/pmon-all/include/pflash.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: /home/knoppix/pmon-all/Targets/Bonito2emini/dev/pflash_tgt.h
/home/knoppix/pmon-all/pmon/dev/flashdev.o: ./flash.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: /home/knoppix/pmon-all/include/pflash.h
/home/knoppix/pmon-all/pmon/dev/flash_amd.o: /home/knoppix/pmon-all/Targets/Bonito2emini/dev/pflash_tgt.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: /home/knoppix/pmon-all/include/pflash.h
/home/knoppix/pmon-all/pmon/dev/flash_int.o: /home/knoppix/pmon-all/Targets/Bonito2emini/dev/pflash_tgt.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: /home/knoppix/pmon-all/include/pflash.h
/home/knoppix/pmon-all/pmon/dev/flash_sst.o: /home/knoppix/pmon-all/Targets/Bonito2emini/dev/pflash_tgt.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: /home/knoppix/pmon-all/include/pflash.h
/home/knoppix/pmon-all/pmon/dev/flash_st.o: /home/knoppix/pmon-all/Targets/Bonito2emini/dev/pflash_tgt.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: ./machine/pio.h ./machine/bus.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/dev/ns16550.o: /home/knoppix/pmon-all/pmon/dev/ns16550.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./machine/cpu.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./machine/psl.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./machine/frame.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./machine/regnum.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./machine/trap.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/arch/mips/machdep.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/signal.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/cpu.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/psl.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/intr.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/param.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/limits.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/stdarg.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/va-mips.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/pio.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/bus.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./target/bonito.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/arch/mips/mips_machdep.o: /home/knoppix/pmon-all/include/autoconf.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: ./machine/cdefs.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: ./machine/ansi.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/include/ctype.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: ./machine/types.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: ./machine/endian.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: ./machine/frame.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: ./target/pmon_target.h
/home/knoppix/pmon-all/pmon/arch/mips/disassemble.o: ./target/bonito.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/types.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/cdefs.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/ansi.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/endian.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/signal.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/cpu.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/psl.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/intr.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/param.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/limits.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/stdarg.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/va-mips.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/bus.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/pio.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./target/pmon_target.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./target/bonito.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/sys/dev/pci/nppbreg.h
/home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.o: /home/knoppix/pmon-all/Targets/Bonito2emini/include/bonito.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/types.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/cdefs.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/ansi.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/endian.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/signal.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/cpu.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/psl.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/intr.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/param.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/limits.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/pio.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/bus.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./target/pmon_target.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./target/bonito.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/include/termio.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/include/queue.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/include/string.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/include/time.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/include/stdlib.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/dev/ic/mc146818reg.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/sys/linux/io.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/include/autoconf.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/include/pflash.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/Targets/Bonito2emini/dev/pflash_tgt.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/Targets/Bonito2emini/include/bonito.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/pmon/dev/gt64240reg.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/pmon/dev/ns16550.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: mod_x86emu_int10.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: mod_vgacon.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: ./flash.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/mycmd.c
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: target/sbd.h
/home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/tgt_machdep.o: /home/knoppix/pmon-all/Targets/Bonito2emini/Bonito/serial.c
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/types.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/cdefs.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/ansi.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/endian.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/signal.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/cpu.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/psl.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/intr.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/param.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/limits.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/stdarg.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/va-mips.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/conf.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/tty.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/termios.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/ttydefaults.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/sys/sys/select.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/include/autoconf.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/bus.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: ./machine/pio.h
/home/knoppix/pmon-all/Targets/Bonito2emini/dev/localbus.o: /home/knoppix/pmon-all/pmon/dev/gt64240reg.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/net/if_media.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/dev/mii/mii.h
/home/knoppix/pmon-all/sys/dev/mii/mii.o: /home/knoppix/pmon-all/sys/dev/mii/miivar.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/cpu.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/psl.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/net/if_media.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/dev/mii/mii.h
/home/knoppix/pmon-all/sys/dev/mii/mii_physubr.o: /home/knoppix/pmon-all/sys/dev/mii/miivar.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/net/if_media.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/dev/mii/mii.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/dev/mii/miivar.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/dev/mii/miidevs.h
/home/knoppix/pmon-all/sys/dev/mii/inphy.o: /home/knoppix/pmon-all/sys/dev/mii/inphyreg.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/include/stdarg.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/Targets/Bonito2emini/include/pmon_target.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./target/bonito.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./target/pmon_target.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/dev/pci/pcidevs.h
/home/knoppix/pmon-all/sys/dev/pci/pciconf.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./target/pmon_target.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./target/bonito.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/sys/dev/pci/pci_map.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./target/pmon_target.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./target/bonito.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/sys/dev/pci/pci.o: /home/knoppix/pmon-all/sys/dev/pci/pcidevs.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: bpfilter.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/net/if_dl.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/net/if_media.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/net/if_types.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/netinet/if_ether.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/dev/mii/miivar.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./target/pmon_target.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./target/bonito.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/dev/pci/pcidevs.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/dev/pci/if_fxpreg.h
/home/knoppix/pmon-all/sys/dev/pci/if_fxp.o: /home/knoppix/pmon-all/sys/dev/pci/if_fxpvar.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: bpfilter.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/mbuf.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/socket.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/net/if.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/net/if_arp.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/net/if_dl.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/net/if_media.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/net/if_types.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/netinet/in.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/netinet/in_systm.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/netinet/in_var.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/netinet/ip.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/netinet/if_ether.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/dev/mii/miivar.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./target/pmon_target.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./target/bonito.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/dev/pci/pcidevs.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/dev/pci/if_fxpreg.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/dev/pci/if_fxpvar.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/dev/pci/etherboot.h
/home/knoppix/pmon-all/sys/dev/pci/rtl8139.o: /home/knoppix/pmon-all/sys/dev/pci/nic.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/vmparam.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/vm/vm_kern.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./target/pmon_target.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./target/bonito.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/pci/pcidevs.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/pci/pciidereg.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/pci/pciidevar.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/pci/pciide_piix_reg.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/pci/pciide_cmd_reg.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/pci/pciide_acer_reg.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/pci/pciide_pdc202xx_reg.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/ata/atavar.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/ic/wdcreg.h
/home/knoppix/pmon-all/sys/dev/pci/pciide.o: /home/knoppix/pmon-all/sys/dev/ic/wdcvar.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: /home/knoppix/pmon-all/include/pmon.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: ./machine/pmon_arch.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: /home/knoppix/pmon-all/include/progress.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: ./target/pmon_target.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: ./target/bonito.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: /home/knoppix/pmon-all/include/stdio.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: /home/knoppix/pmon-all/sys/dev/pci/pcivar.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: /home/knoppix/pmon-all/Targets/Bonito2emini/pci/pci_machdep.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: /home/knoppix/pmon-all/sys/dev/pci/pcireg.h
/home/knoppix/pmon-all/sys/dev/pci/pcidevs.o: /home/knoppix/pmon-all/sys/dev/pci/pcidevs.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: ./machine/param.h ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/conf.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/stat.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/ioctl.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/ttycom.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/ioccom.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/dkio.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/filio.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/sockio.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/buf.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/disklabel.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: ./machine/disklabel.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/dkbad.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/disk.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/lock.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/vnode.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/vm/vm.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/vm/vm_param.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: ./machine/vmparam.h ./machine/bus.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: ./machine/pio.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/dev/ata/atareg.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/dev/ata/atavar.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/dev/ata/wdvar.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/dev/ic/wdcreg.h
/home/knoppix/pmon-all/sys/dev/ata/wd.o: /home/knoppix/pmon-all/sys/sys/ataio.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/cdefs.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/signal.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/cpu.h ./machine/psl.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/stat.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/buf.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/disklabel.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/disklabel.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/dkbad.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/sys/proc.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: ./machine/bus.h ./machine/pio.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/dev/ata/atareg.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/dev/ata/atavar.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/dev/ic/wdcreg.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/dev/ic/wdcvar.h
/home/knoppix/pmon-all/sys/dev/ata/ata_wdc.o: /home/knoppix/pmon-all/sys/dev/ata/wdvar.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/param.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/types.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: ./machine/types.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/cdefs.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: ./machine/cdefs.h ./machine/ansi.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: ./machine/endian.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/endian.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/simplelock.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/syslimits.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/errno.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/time.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/resource.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/ucred.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/uio.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/signal.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: ./machine/signal.h ./machine/cpu.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: ./machine/psl.h ./machine/intr.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/siginfo.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: ./machine/param.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: ./machine/limits.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/systm.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: ./machine/stdarg.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: ./machine/va-mips.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/kernel.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/file.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/fcntl.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/unistd.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/queue.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/stat.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/malloc.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/device.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/sys/syslog.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/dev/ic/wdcreg.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/dev/ata/atareg.h
/home/knoppix/pmon-all/sys/dev/ata/ata.o: /home/knoppix/pmon-all/sys/dev/ata/atavar.h
