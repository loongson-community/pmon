ifndef PMONCC
CC=$(CROSS_COMPILE)gcc -mabi=32
CROSS_COMPILE ?= mipsel-linux-
else
CC=${PMONCC}
endif


ifeq "$(shell echo ${CC}|grep  'mips-elf-')" ""
 ifeq "$(shell echo ${CC}|grep  '\-mabi=64')" ""
LD		= $(CROSS_COMPILE)ld  -m elf32ltsmip -G 0 -static -n -nostdlib
OUT_FORMAT="elf32-tradlittlemips"
 else
LD		= $(CROSS_COMPILE)ld  -m elf64ltsmip -G 0 -static -n -nostdlib
OUT_FORMAT="elf64-tradlittlemips"
 endif
else
ENDIAN_FLAG= -EL
LD		= $(CROSS_COMPILE)ld  -m elf32ebmip -G 0 -static -n -nostdlib -EL
CC :=${CC} -EL
OUT_FORMAT="elf32-littlemips"
endif

GZROMSTARTADDR?=0xffffffff8f900000
ROMSTARTADDR?=0xffffffff8f010000
RAMSTARTADDR?=0xffffffff88000000
export CROSS_COMPILE
export CC += ${MYCC}  -mno-abicalls -fno-pic
export LD
export MKDEP=makedepend
ifeq ("${tgt}","rom")
gencode=./genrom
endif

ifeq ("${tgt}","ejtag_rom")
gencode=./genrom_ejtag
endif

ifeq ("${tgt}","ejtag_rom1")
gencode=./genrom
CC += -DBOOT_FROM_EJTAG
endif

ifeq ("${tgt}","ejtag_ram")
gencode=./genrom
ROMSTARTADDR=0xffffffffff200200
endif

ifeq ("${tgt}","rom1")
gencode=./genrom
endif


ifdef obj
export CC += -g3
pwd=$(shell pwd)
gcc_E:
	cd ../Targets/${TARGET}/compile/${TARGETEL};${pwd}/mymake  ${obj}
endif

ifndef tgt
help:
	@echo use make tgt=sim for sim in linux
	@echo use make tgt=rom for generate romfile gzrom.bin
	@echo use make tgt=ram for generate ramfile gzram
	@echo use make cfg     for config
	@echo use make recfg   for change config
	@echo use make tgt=rom DEBUG=-g MYCC="'"-g3 -DMYDBG='"printf(\"debug:%s,%d\\n\",__FILE__,__LINE__);"'"'" to support MYDBG macro. 
else
all: ${tgt}
sim: clean pmon.bin.c
	gcc -o zloader  -DSIM zloader.c
ejtag_rom ejtag_rom1 ejtag_ram rom: clean ${START} zloader.o 
	gcc  -DSTARTADDR=${GZROMSTARTADDR} -DOUT_FORMAT=\"${OUT_FORMAT}\" -DOUT_ARCH=mips -Umips -E -P ld.script.S  > ld.script
	${LD} -T ld.script -g -e start -o gzrom ${START} zloader.o 
	${CROSS_COMPILE}objcopy -O binary gzrom gzrom.bin

ram: clean startram.o zloader.o 
	${CC}  -DSTARTADDR=${GZROMSTARTADDR} -DOUT_FORMAT=\"${OUT_FORMAT}\" -DOUT_ARCH=mips -Umips -E -P ld.script.S  > ld.script
	${LD}  -T ld.script -e start -o gzram startram.o zloader.o 

${START}:
	rm -f ../Targets/${TARGET}/compile/${TARGETEL}/${START}
	gcc  -DSTARTADDR=${ROMSTARTADDR} -DOUT_FORMAT=\"${OUT_FORMAT}\" -DOUT_ARCH=mips -Umips -E -P ld.script.S  > ../Targets/${TARGET}/conf/ld.script
	make -C ../Targets/${TARGET}/compile/${TARGETEL}/
	cp ../Targets/${TARGET}/compile/${TARGETEL}/${START} .

startram.o:
	gcc  -DSTARTADDR=${ROMSTARTADDR} -DOUT_FORMAT=\"${OUT_FORMAT}\" -DOUT_ARCH=mips -Umips -E -P ld.script.S  > ../Targets/${TARGET}/conf/ld.script
	make -C ../Targets/${TARGET}/compile/${TARGETEL}/ DEBUG='-g -DMYDBG="printf(\"debug:%s,%d\\n\",__FILE__,__LINE__);"'
	$(CC) -D_LOCORE -G 0  -nostdinc   -DMIPS  -DCONS_BAUD="B115200"  -DCONFIG_CACHE_64K_4WAY  -D_KERNEL -D__OpenBSD__ -DPMON -D__PMON__ -mno-abicalls -c startram.S ${ZLOADER_OPTIONS} -D MEMSIZE=${MEMSIZE}
# $(CC) -D_LOCORE -G 0  -nostdinc   -DMIPS  -DCONS_BAUD="B115200"  -DCONFIG_CACHE_64K_4WAY  -D_KERNEL -D__OpenBSD__ -DPMON -D__PMON__ -EL -mno-abicalls -mcpu=r4000 -c startram.S -D MEMSIZE=${MEMSIZE}
zloader.o: zloader.c inflate.c  malloc.c  memop.c  pmon.bin.c initmips.c
	$(CC) -c zloader.c ${ZLOADER_OPTIONS} -DMEMSIZE=${MEMSIZE}

rom1: clean ${START} zloader1.o 
	gcc  -DSTARTADDR=0xffffffff9fc00000 -D_ROM1 -DOUT_FORMAT=\"${OUT_FORMAT}\" -DOUT_ARCH=mips -Umips -E -P ld.script.S  > ld.script
	${LD} -T ld.script -e start -o gzrom ${START} zloader.o 
	${CROSS_COMPILE}objcopy -O binary gzrom gzrom.bin

zloader1.o: zloader.c inflate.c  malloc.c  memop.c  pmon.bin.c initmips.c program.o xmodem.o
	sed -i 's/initmips/initmips1/' initmips.c
	sed -i 's/flush_cache2();//;s/flush_cache();//' initmips.c
	$(CC) -g -c -o zloader.tmp.o zloader.c ${ZLOADER_OPTIONS} -DMEMSIZE=${MEMSIZE}
	${LD}  -r -o zloader.o zloader.tmp.o program.o xmodem.o

program.o: program.S
	make -C  ../Targets/${TARGET}/compile/${TARGETEL}/ `pwd`/$@

xmodem.o: xmodem.c
	make -C  ../Targets/${TARGET}/compile/${TARGETEL}/ `pwd`/$@

zlib_loader.o: zlib_loader.c  ./zlib_gzip zlib_pmon.bin.c initmips.c
	cd zlib_inflate;$(CC) -I ../../include -c *.c
	#cd zlib_inflate;$(CC) -I ../../include -EL -mcpu=r4000 -c *.c
	#$(CC) -I ../include -EL -mcpu=r4000 -c zlib_loader.c 
	$(CC) -I ../include -c zlib_loader.c 

initmips.c:  ../Targets/${TARGET}/compile/${TARGETEL}/pmon
	${gencode} $< > initmips.c

zlib_pmon.bin.c:  ../Targets/${TARGET}/compile/${TARGETEL}/pmon.bin
	./zlib_gzip $<  zlib_pmon.bin.gz
	./bin2c zlib_pmon.bin.gz zlib_pmon.bin.c biosdata
	
pmon.bin.c:  ../Targets/${TARGET}/compile/${TARGETEL}/pmon.bin
	gzip $< -c > pmon.bin.gz
	./bin2c pmon.bin.gz pmon.bin.c biosdata

endif

ifeq ("${tgt}","rom")
gencode=./genrom
endif

ifeq ("${tgt}","zlib_rom")
gencode=./genrom
endif


ifndef gencode
gencode=./genram
endif

dtb:
	make -C ../Targets/${TARGET}/compile/${TARGETEL}/ DTB_O=`pwd`/${TARGETEL}.dtb.i DTB_I=`pwd`/../Targets/${TARGET}/conf/${TARGETEL}.dts dtb
	./dtc -I dts -O dtb -o ${TARGETEL}.dtb  ${TARGETEL}.dtb.i
	[ -f gzrom.bin ] && cp gzrom.bin gzrom-dtb.bin && python ../tools/pmonenv.py -f gzrom-dtb.bin -d ${TARGETEL}.dtb -w  al=\(usb0,0\)/boot/vmlinuz al1=\(wd0,0\)/boot/vmlinuz append="'console=ttyS0,115200 console=tty initcall_debug=1 loglevel=20'" FR=1
cleanall: clean
	make -C ../Targets/${TARGET}/compile/${TARGETEL}/ clean
clean:
	rm -rf *.o zlib_gzip zloader pmon.bin.c gzrom gzrom.bin gzram  initmips.c pmon.bin.gz zlib_deflate/*.o zlib_inflate/*.o zlib_gzrom zlib_gzrom.bin
cfg:
	# DO NOT DELETE
	perl -i -ne 'print;exit if(/^# DO NOT DELETE/);' ../lib/libc/Makefile 
	perl -i -ne 'print;exit if(/^# DO NOT DELETE/);' ../lib/libm/Makefile 
	perl -i -ne 'print;exit if(/^# DO NOT DELETE/);' ../lib/libz/Makefile 
	mkdir -p ../Targets/${TARGET}/compile
	cd ../Targets/${TARGET}/conf/;pmoncfg ${TARGETEL}
	make -C ../Targets/${TARGET}/compile/${TARGETEL}/ depend clean
recfg:
	vi ../Targets/${TARGET}/conf/$(TARGETEL)
	make cfg

zlib_gzip: zlib_gzip.c
	cd zlib_deflate/;gcc -I ../../include -c *.c
	cd zlib_inflate/;gcc -I ../../include -c *.c
	gcc -o zlib_gzip zlib_gzip.c zlib_deflate/*.o zlib_inflate/*.o
