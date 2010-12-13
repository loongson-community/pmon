IDENT=-DVGAROM_IN_BIOS -DSYSTYPE="\"Bonito\"" -DTARGETNAME="\"Bonito\"" -DBONITOEL -DDEVBD2F -DLS3_SERVER -DMIPS -DINET -DLS3_HT -DENABLE_SATA -Dloongson3A3 -DSERVER_3A -DRS780E -DUSE_780E_VGA -DVGA_NO_ROM -DVGA_BASE="0xbe000000" -DSHARED_VRAM="0x40" -DGPU_UMA -DVESAFB -DCONS_BAUD="B115200" -DMY40IO -DNOPCINAMES -DNOSNOOP -DHAVE_TOD -DHAVE_NVENV -DHAVE_LOGO -DUSE_SUPERIO_UART -DAUTOLOAD -DCONFIG_CACHE_64K_4WAY -DNVRAM_IN_FLASH -DIDECD -DHAVE_NB_SERIAL -DFOR_GXEMUL -DFLOATINGPT -DPCI_IDSEL_SB700="0xe" -DWDC_NORESET -DINPUT_FROM_BOTH -DOUTPUT_TO_BOTH -DDEVBD2F_VIA
S!=	echo `/bin/pwd`/../../../..
ENDIAN=EL
#	$Id: Makefile.Bonito,v 1.1.1.1 2006/06/29 06:43:25 cpu Exp $
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

ifeq ($(PMON64),1)
CROSS_COMPILE 	=mipsel-linux-
else
CROSS_COMPILE 	=mips-elf-
endif

#
# Include the make variables (CC, etc...)
#

AS		= $(CROSS_COMPILE)as
ifeq ($(PMON64),1)
LD		= $(CROSS_COMPILE)ld -m elf64ltsmip -G 0 -static -n -nostdlib
else
LD		= $(CROSS_COMPILE)ld 
endif
#CC		= $(CROSS_COMPILE)gcc
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

TARGET= ${S}/Targets/Bonito3a780e
MACHINE=mips
MACHINE_ARCH=mips
COMPILEDIR=${shell pwd}
OBJDIR=${COMPILEDIR}
PMONDIR=${S}


INCLUDES=	-I. -I${S}/include -I./machine -I${S} \
		-I${S}/x86emu/int10/x86emu/include -I${S}/x86emu/int10/x86emu/src/x86emu/x86emu \
		-I${S}/sys/arch/${MACHINE}/include -I${S}/sys \
		-I${TARGET} -I${COMPILEDIR} -nostdinc -I${S}/x86emu/src/x86emu -I${S}/x86emu/src/x86emu/include
#CPPFLAGS=	${INCLUDES} ${IDENT} -D_KERNEL -D__OpenBSD__ -DPMON -D__PMON__\
#		-${ENDIAN} -mno-abicalls -mips3 -mmemcpy -mcpu=r4000
ifeq ($(PMON64),1)
CPPFLAGS=	${INCLUDES} ${IDENT} -D_KERNEL -D__OpenBSD__ -DPMON -D__PMON__\
		-mno-abicalls -mips3 -mmemcpy 
else
CPPFLAGS=	${INCLUDES} ${IDENT} -D_KERNEL -D__OpenBSD__ -DPMON -D__PMON__\
		-$(ENDIAN) -mno-abicalls -mips3 -mmemcpy 
endif
#CWARNFLAGS=	-Werror -Wall -Wmissing-prototypes -Wstrict-prototypes \
		-Wno-uninitialized -Wno-format -Wno-main
CWARNFLAGS=	-Wall -Wstrict-prototypes \
		-Wno-uninitialized -Wno-format -Wno-main
ifeq ($(PMON64),1)
CFLAGS=		${DEBUG} ${CWARNFLAGS} ${OPT} -G 0 -fno-pic -fno-builtin -mabi=64
AFLAGS=		-D_LOCORE -G 0 -fno-pic -fno-builtin -mabi=64
else
CFLAGS=		${DEBUG} ${CWARNFLAGS} ${OPT} -G 0
AFLAGS=		-D_LOCORE -G 0 
endif
LFLAGS=		-${ENDIAN} -N -G 0 -T../../conf/ld.script -e start
STRIPFLAGS=	-g -S --strip-debug

HOSTCC?=	${CC}
HOSTED_CPPFLAGS=${CPPFLAGS:S/^-nostdinc$//}
HOSTED_CFLAGS=	${CFLAGS}

include ${S}/lib/libc/Makefile.inc
LIBC=${CLIB}
include ${S}/lib/libm/Makefile.inc
LIBM=${MLIB}
include ${S}/lib/libz/Makefile.inc
LIBZ=${ZLIB}

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
	ip_output.o raw_ip.o udp_usrreq.o tcp_debug.o tcp_input.o \
	tcp_output.o tcp_subr.o tcp_timer.o tcp_usrreq.o kern_clock.o \
	kern_misc.o kern_synch.o uipc_domain.o uipc_socket2.o \
	kern_descrip.o kern_proc.o kern_syscall.o sys_generic.o \
	subr_autoconf.o uipc_mbuf.o uipc_syscalls.o init_main.o \
	kern_malloc.o kern_sig.o kern_time.o sys_socket.o uipc_socket.o \
	bus_dma.o boot.o mycmd.o xyzModem.o crc16.o newmt.o cmdtable.o \
	debuggercmds.o devls.o cmd_go.o hist.o ifaddr.o load.o tftpd.o \
	cmd_grub.o memcmds.o memtst.o miscmds.o pcicmds.o pflash.o \
	cmd_env.o transp.o sym.o time.o shellcmds.o command.o spacetest.o \
	bootkernel.o cmd_ls.o cat.o about.o autoconf.o callvec.o \
	cmdparser.o debugger.o exception.o main.o more.o rsa.o sbrk.o \
	env.o ramfile.o termio.o socket.o gzip.o iso9660fs.o devfs.o \
	diskfs.o ext2fs.o dev_part.o loadfn.o exec.o exec_bin.o \
	exec_elf.o exec_elf64.o exec_srec.o exec_txt.o netio.o bootp.o \
	ifconfig.o ping.o tftplib.o httplib.o rs690_struct.o debug.o \
	decode.o fpu.o ops.o ops2.o prim_ops.o sys.o generic.o \
	helper_exec.o helper_mem.o xf86int10.o xf86x86emu.o vesafb.o \
	cfb_console.o gunzip.o zlib.o kbd.o vgacon.o generic_poll.o \
	flash.o flashdev.o flash_amd.o flash_int.o flash_sst.o ns16550.o \
	machdep.o mips_machdep.o disassemble.o mips.o cache.o usb.o \
	usb_storage.o part.o usb-ohci.o usb_kbd.o cmd_usb.o usb_uhci.o \
	loopdev.o md5.o pci_machdep.o tgt_machdep.o rs780_cmn.o \
	amd_780e.o sb700.o sb700_ide.o sb700_lpc.o sb700_sata.o \
	sb700_usb.o localbus.o mii.o mii_physubr.o inphy.o pciconf.o \
	pci_map.o pci.o if_fxp.o rtl8168.o pciide.o ppb.o pcidevs.o wd.o \
	ata_wdc.o ata.o cd.o

CFILES=	$S/sys/dev/ic/wdc.c $S/pmon/dev/mainbus.c $S/pmon/dev/pcibr.c \
	$S/sys/net/if.c $S/sys/net/if_ethersubr.c $S/sys/net/if_loop.c \
	$S/sys/net/if_media.c $S/sys/net/radix.c $S/sys/net/raw_cb.c \
	$S/sys/net/raw_usrreq.c $S/sys/net/route.c $S/sys/net/rtsock.c \
	$S/sys/netinet/if_ether.c $S/sys/netinet/in.c \
	$S/sys/netinet/in_cksum.c $S/sys/netinet/in_pcb.c \
	$S/sys/netinet/in_proto.c $S/sys/netinet/ip_icmp.c \
	$S/sys/netinet/ip_id.c $S/sys/netinet/ip_input.c \
	$S/sys/netinet/ip_output.c $S/sys/netinet/raw_ip.c \
	$S/sys/netinet/udp_usrreq.c $S/sys/netinet/tcp_debug.c \
	$S/sys/netinet/tcp_input.c $S/sys/netinet/tcp_output.c \
	$S/sys/netinet/tcp_subr.c $S/sys/netinet/tcp_timer.c \
	$S/sys/netinet/tcp_usrreq.c $S/sys/kern/kern_clock.c \
	$S/sys/kern/kern_misc.c $S/sys/kern/kern_synch.c \
	$S/sys/kern/uipc_domain.c $S/sys/kern/uipc_socket2.c \
	$S/sys/kern/kern_descrip.c $S/sys/kern/kern_proc.c \
	$S/sys/kern/kern_syscall.c $S/sys/kern/sys_generic.c \
	$S/sys/kern/subr_autoconf.c $S/sys/kern/uipc_mbuf.c \
	$S/sys/kern/uipc_syscalls.c $S/sys/kern/init_main.c \
	$S/sys/kern/kern_malloc.c $S/sys/kern/kern_sig.c \
	$S/sys/kern/kern_time.c $S/sys/kern/sys_socket.c \
	$S/sys/kern/uipc_socket.c $S/sys/dev/bus_dma.c \
	$S/pmon/cmds/boot.c $S/pmon/cmds/mycmd.c $S/pmon/cmds/xyzModem.c \
	$S/pmon/cmds/crc16.c $S/pmon/cmds/newmt/newmt.c \
	$S/pmon/cmds/cmdtable.c $S/pmon/cmds/debuggercmds.c \
	$S/pmon/cmds/devls.c $S/pmon/cmds/cmd_go.c $S/pmon/cmds/hist.c \
	$S/pmon/cmds/ifaddr.c $S/pmon/cmds/load.c $S/pmon/cmds/tftpd.c \
	$S/pmon/cmds/cmd_grub.c $S/pmon/cmds/memcmds.c \
	$S/pmon/cmds/memtst.c $S/pmon/cmds/miscmds.c \
	$S/pmon/cmds/pcicmds.c $S/pmon/cmds/pflash.c \
	$S/pmon/cmds/cmd_env.c $S/pmon/cmds/transp.c $S/pmon/cmds/sym.c \
	$S/pmon/cmds/time.c $S/pmon/cmds/shellcmds.c \
	$S/pmon/cmds/command.c $S/pmon/cmds/spacetest.c \
	$S/pmon/common/bootkernel.c $S/pmon/cmds/cmd_ls.c \
	$S/pmon/cmds/cat.c $S/pmon/common/about.c \
	$S/pmon/common/autoconf.c $S/pmon/common/callvec.c \
	$S/pmon/common/cmdparser.c $S/pmon/common/debugger.c \
	$S/pmon/common/exception.c $S/pmon/common/main.c \
	$S/pmon/common/more.c $S/pmon/common/rsa.c $S/pmon/common/sbrk.c \
	$S/pmon/common/env.c $S/pmon/fs/ramfile.c $S/pmon/fs/termio.c \
	$S/pmon/fs/socket.c $S/pmon/fs/gzip.c $S/pmon/fs/iso9660fs.c \
	$S/pmon/fs/devfs.c $S/pmon/fs/diskfs.c $S/pmon/fs/ext2fs.c \
	$S/pmon/fs/dev_part.c $S/pmon/loaders/loadfn.c \
	$S/pmon/loaders/exec.c $S/pmon/loaders/exec_bin.c \
	$S/pmon/loaders/exec_elf.c $S/pmon/loaders/exec_elf64.c \
	$S/pmon/loaders/exec_srec.c $S/pmon/loaders/exec_txt.c \
	$S/pmon/netio/netio.c $S/pmon/netio/bootp.c \
	$S/pmon/netio/ifconfig.c $S/pmon/netio/ping.c \
	$S/pmon/netio/tftplib.c $S/pmon/netio/httplib.c \
	$S/x86emu/int10/rs690_struct.c \
	$S/x86emu/int10/x86emu/src/x86emu/debug.c \
	$S/x86emu/int10/x86emu/src/x86emu/decode.c \
	$S/x86emu/int10/x86emu/src/x86emu/fpu.c \
	$S/x86emu/int10/x86emu/src/x86emu/ops.c \
	$S/x86emu/int10/x86emu/src/x86emu/ops2.c \
	$S/x86emu/int10/x86emu/src/x86emu/prim_ops.c \
	$S/x86emu/int10/x86emu/src/x86emu/sys.c $S/x86emu/int10/generic.c \
	$S/x86emu/int10/helper_exec.c $S/x86emu/int10/helper_mem.c \
	$S/x86emu/int10/xf86int10.c $S/x86emu/int10/xf86x86emu.c \
	$S/x86emu/int10/vesafb.c $S/fb/cfb_console.c $S/fb/gunzip.c \
	$S/fb/zlib.c $S/pmon/dev/kbd.c $S/pmon/dev/vgacon.c \
	$S/pmon/dev/generic_poll.c $S/pmon/dev/flash.c \
	$S/pmon/dev/flashdev.c $S/pmon/dev/flash_amd.c \
	$S/pmon/dev/flash_int.c $S/pmon/dev/flash_sst.c \
	$S/pmon/dev/ns16550.c $S/pmon/arch/mips/machdep.c \
	$S/pmon/arch/mips/mips_machdep.c $S/pmon/arch/mips/disassemble.c \
	$S/sys/dev/usb/usb.c $S/sys/dev/usb/usb_storage.c \
	$S/sys/dev/usb/part.c $S/sys/dev/usb/usb-ohci.c \
	$S/sys/dev/usb/usb_kbd.c $S/sys/dev/usb/cmd_usb.c \
	$S/sys/dev/usb/usb_uhci.c $S/pmon/dev/loopdev.c $S/pmon/dev/md5.c \
	$S/Targets/Bonito3a780e/pci/pci_machdep.c \
	$S/Targets/Bonito3a780e/Bonito/tgt_machdep.c \
	$S/Targets/Bonito3a780e/pci/rs780_cmn.c \
	$S/Targets/Bonito3a780e/pci/amd_780e.c \
	$S/Targets/Bonito3a780e/pci/sb700.c \
	$S/Targets/Bonito3a780e/pci/sb700_ide.c \
	$S/Targets/Bonito3a780e/pci/sb700_lpc.c \
	$S/Targets/Bonito3a780e/pci/sb700_sata.c \
	$S/Targets/Bonito3a780e/pci/sb700_usb.c \
	$S/Targets/Bonito3a780e/dev/localbus.c $S/sys/dev/mii/mii.c \
	$S/sys/dev/mii/mii_physubr.c $S/sys/dev/mii/inphy.c \
	$S/sys/dev/pci/pciconf.c $S/sys/dev/pci/pci_map.c \
	$S/sys/dev/pci/pci.c $S/sys/dev/pci/if_fxp.c \
	$S/sys/dev/pci/rtl8168.c $S/sys/dev/pci/pciide.c \
	$S/sys/dev/pci/ppb.c $S/sys/dev/pci/pcidevs.c $S/sys/dev/ata/wd.c \
	$S/sys/dev/ata/ata_wdc.c $S/sys/dev/ata/ata.c $S/sys/dev/ata/cd.c

SFILES=

# load lines for config "xxx" will be emitted as:
# xxx: ${SYSTEM_DEP}
#	${SYSTEM_LD_HEAD}
#	${SYSTEM_LD}
#	${SYSTEM_LD_TAIL}
SYSTEM_OBJ=	start.o crtbegin.o param.o ioconf.o ri.o ${OBJS} ${LIBC} ${LIBM} ${LIBZ} \
		crtend.o
SYSTEM_DEP=	Makefile ${SYSTEM_OBJ}
SYSTEM_LD_HEAD=	rm -f $@
ifeq ($(PMON64),1)
SYSTEM_LD=	@echo ${LD} ${LFLAGS} -o $@ ${LIBDIR} '$${SYSTEM_OBJ}' vers.o; \
		${LD} ${LFLAGS} -o $@ ${LIBDIR} ${SYSTEM_OBJ} vers.o -L. -lgcc
else
SYSTEM_LD=	@echo ${LD} ${LFLAGS} -o $@ ${LIBDIR} '$${SYSTEM_OBJ}' vers.o; \
		${LD} ${LFLAGS} -o $@ ${LIBDIR} ${SYSTEM_OBJ} vers.o -L. 
endif
SYSTEM_LD_TAIL=	@${SIZE} $@; chmod 755 $@ ; \
		${OBJCOPY} -O binary $@ $@.bin

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

zpmon: startz.o
	rm start.o && cp -f startz.o start.o
	make pmon
	make -C ../zboot zpmon
startz.o: ${TARGET}/Bonito/startz.S Makefile
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

tcp_debug.o: $S/sys/netinet/tcp_debug.c
	${NORMAL_C}

tcp_input.o: $S/sys/netinet/tcp_input.c
	${NORMAL_C}

tcp_output.o: $S/sys/netinet/tcp_output.c
	${NORMAL_C}

tcp_subr.o: $S/sys/netinet/tcp_subr.c
	${NORMAL_C}

tcp_timer.o: $S/sys/netinet/tcp_timer.c
	${NORMAL_C}

tcp_usrreq.o: $S/sys/netinet/tcp_usrreq.c
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

mycmd.o: $S/pmon/cmds/mycmd.c
	${NORMAL_C}

xyzModem.o: $S/pmon/cmds/xyzModem.c
	${NORMAL_C}

crc16.o: $S/pmon/cmds/crc16.c
	${NORMAL_C}

newmt.o: $S/pmon/cmds/newmt/newmt.c
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

load.o: $S/pmon/cmds/load.c
	${NORMAL_C}

tftpd.o: $S/pmon/cmds/tftpd.c
	${NORMAL_C}

cmd_grub.o: $S/pmon/cmds/cmd_grub.c
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

transp.o: $S/pmon/cmds/transp.c
	${NORMAL_C}

sym.o: $S/pmon/cmds/sym.c
	${NORMAL_C}

time.o: $S/pmon/cmds/time.c
	${NORMAL_C}

shellcmds.o: $S/pmon/cmds/shellcmds.c
	${NORMAL_C}

command.o: $S/pmon/cmds/command.c
	${NORMAL_C}

spacetest.o: $S/pmon/cmds/spacetest.c
	${NORMAL_C}

bootkernel.o: $S/pmon/common/bootkernel.c
	${NORMAL_C}

cmd_ls.o: $S/pmon/cmds/cmd_ls.c
	${NORMAL_C}

cat.o: $S/pmon/cmds/cat.c
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

gzip.o: $S/pmon/fs/gzip.c
	${NORMAL_C}

iso9660fs.o: $S/pmon/fs/iso9660fs.c
	${NORMAL_C}

devfs.o: $S/pmon/fs/devfs.c
	${NORMAL_C}

diskfs.o: $S/pmon/fs/diskfs.c
	${NORMAL_C}

ext2fs.o: $S/pmon/fs/ext2fs.c
	${NORMAL_C}

dev_part.o: $S/pmon/fs/dev_part.c
	${NORMAL_C}

loadfn.o: $S/pmon/loaders/loadfn.c
	${NORMAL_C}

exec.o: $S/pmon/loaders/exec.c
	${NORMAL_C}

exec_bin.o: $S/pmon/loaders/exec_bin.c
	${NORMAL_C}

exec_elf.o: $S/pmon/loaders/exec_elf.c
	${NORMAL_C}

exec_elf64.o: $S/pmon/loaders/exec_elf64.c
	${NORMAL_C}

exec_srec.o: $S/pmon/loaders/exec_srec.c
	${NORMAL_C}

exec_txt.o: $S/pmon/loaders/exec_txt.c
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

httplib.o: $S/pmon/netio/httplib.c
	${NORMAL_C}

rs690_struct.o: $S/x86emu/int10/rs690_struct.c
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

vesafb.o: $S/x86emu/int10/vesafb.c
	${NORMAL_C}

cfb_console.o: $S/fb/cfb_console.c
	${NORMAL_C}

gunzip.o: $S/fb/gunzip.c
	${NORMAL_C}

zlib.o: $S/fb/zlib.c
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

usb.o: $S/sys/dev/usb/usb.c
	${NORMAL_C}

usb_storage.o: $S/sys/dev/usb/usb_storage.c
	${NORMAL_C}

part.o: $S/sys/dev/usb/part.c
	${NORMAL_C}

usb-ohci.o: $S/sys/dev/usb/usb-ohci.c
	${NORMAL_C}

usb_kbd.o: $S/sys/dev/usb/usb_kbd.c
	${NORMAL_C}

cmd_usb.o: $S/sys/dev/usb/cmd_usb.c
	${NORMAL_C}

usb_uhci.o: $S/sys/dev/usb/usb_uhci.c
	${NORMAL_C}

loopdev.o: $S/pmon/dev/loopdev.c
	${NORMAL_C}

md5.o: $S/pmon/dev/md5.c
	${NORMAL_C}

pci_machdep.o: $S/Targets/Bonito3a780e/pci/pci_machdep.c
	${NORMAL_C}

tgt_machdep.o: $S/Targets/Bonito3a780e/Bonito/tgt_machdep.c
	${NORMAL_C}

rs780_cmn.o: $S/Targets/Bonito3a780e/pci/rs780_cmn.c
	${NORMAL_C}

amd_780e.o: $S/Targets/Bonito3a780e/pci/amd_780e.c
	${NORMAL_C}

sb700.o: $S/Targets/Bonito3a780e/pci/sb700.c
	${NORMAL_C}

sb700_ide.o: $S/Targets/Bonito3a780e/pci/sb700_ide.c
	${NORMAL_C}

sb700_lpc.o: $S/Targets/Bonito3a780e/pci/sb700_lpc.c
	${NORMAL_C}

sb700_sata.o: $S/Targets/Bonito3a780e/pci/sb700_sata.c
	${NORMAL_C}

sb700_usb.o: $S/Targets/Bonito3a780e/pci/sb700_usb.c
	${NORMAL_C}

localbus.o: $S/Targets/Bonito3a780e/dev/localbus.c
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

rtl8168.o: $S/sys/dev/pci/rtl8168.c
	${NORMAL_C}

pciide.o: $S/sys/dev/pci/pciide.c
	${NORMAL_C}

ppb.o: $S/sys/dev/pci/ppb.c
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

