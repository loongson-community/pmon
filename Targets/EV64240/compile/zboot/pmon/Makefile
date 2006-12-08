CC = mips-elf-gcc
AS = mips-elf-as
LD = mips-elf-ld
OBJDUMP = mips-elf-objdump
OBJCOPY = mips-elf-objcopy
STRIP = mips-elf-strip
SIZE = mips-elf-size
NM = mips-elf-nm

CFLAGS	:= $(CPPFLAGS) -EL -O2 -D__BOOTER__  -fno-builtin\
	-nostdinc -Wno-main -Wno-format\
	-G 0 -mno-abicalls -mcpu=r4000 -mips3 \
		-I.. 
AFLAGS	+= -D__BOOTER__

.c.s:
	$(CC) $(CFLAGS) -S -o $*.s $<
.s.o:
	$(AS) -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) -D__BOOTER__ -c -o $*.o $<
.S.s:
	$(CPP) $(AFLAGS) -o $*.o $<
.S.o:
	$(CC) $(CFLAGS) $(AFLAGS) -c -o $*.o $<

#########################################################################
BNAME = pmon64240
# START BOARD SPECIFIC VARIABLES
# These two variables control where the zImage is stored
# in flash and loaded in memory. If you change either one,
# be sure to make the appropriate change to the zImage
# rule.
RAM_LOAD_ADDR = 0x80010000

# These two variables specify the free ram region
# that can be used for temporary malloc area
AVAIL_RAM_START=0x80400000
AVAIL_RAM_END=0x80800000

# This one must match the LOADADDR in arch/mips/Makefile!
LOADADDR=0x80020000
# END BOARD SPECIFIC VARIABLES
#########################################################################

ZLINKFLAGS = -G 0 -N -T ld.script -Ttext $(RAM_LOAD_ADDR)

OBJECTS := ../../EV64240EL/start.o  \
	../ctype.o ../string.o ../zlib.o ../misc-common.o 

ENTRY := ../utils/entry
OFFSET := ../utils/offset
SIZE := ../utils/size

all: zpmon

clean:
	rm -rf *.o pmon*

head.o: head.S $(TOPDIR)/vmlinux
	$(CC) $(AFLAGS) \
	-c -o $*.o $<

../misc-common.o:
	$(CC) $(CFLAGS) -DZIMAGE_OFFSET=0 \
		-DAVAIL_RAM_START=$(AVAIL_RAM_START) \
		-DAVAIL_RAM_END=$(AVAIL_RAM_END) \
		-DLOADADDR=$(LOADADDR) \
		-DZIMAGE_SIZE=0 -c -o $@ $*.c

# This is the first pass at building the boot loader image,
# without knowing the file offset where the vmlinuz.gz
# kernel will end up.  We build this image, check the offset,
# and then rebuild it with the correct offset and size
# passed to mips-simple.c
zvpmon.no: $(OBJECTS) ../images/pmon.bin.gz
	$(LD) $(ZLINKFLAGS) -o $@.tmp $(OBJECTS) $(LIBS)
	cp $@.tmp test
	$(OBJCOPY) -O binary $@.tmp test.bin
	$(OBJCOPY) -R .comment \
		--add-section=image=../images/pmon.bin.gz \
		$@.tmp $@
	rm -f $@.tmp


# This is the final image we build, now that we know what
# the vmlinuz.gz offset is.
zvpmon: $(OBJECTS) ../images/pmon.bin.gz zvpmon.no
	$(CC) $(CFLAGS) \
		-DZIMAGE_OFFSET=$(shell sh $(OFFSET) $(OBJDUMP) $@.no image) \
		-DZIMAGE_SIZE=$(shell sh $(SIZE) $(OBJDUMP) $@.no image) \
		-D__BOOTER__ \
		-DAVAIL_RAM_START=$(AVAIL_RAM_START) \
		-DAVAIL_RAM_END=$(AVAIL_RAM_END) \
		-DLOADADDR=$(LOADADDR) \
		-c -o ../misc-common.o ../misc-common.c
	$(LD) $(ZLINKFLAGS) -o $@.tmp $(OBJECTS) $(LIBS)
	$(OBJCOPY) -R .comment \
		--add-section=image=../images/pmon.bin.gz \
		$@.tmp $@
	$(OBJCOPY) --adjust-section-vma=image+$(RAM_LOAD_ADDR) $@
	$(OBJCOPY) --adjust-section-vma=image+$(shell sh $(OFFSET) \
	$(OBJDUMP) $@.no image ) $@
	rm -f $@.tmp
	rm -f $@.no


# Here we manipulate the image in order to get it the necessary
# srecord file we need.
zpmon: zvpmon
	mv zvpmon ../images/$@.$(BNAME)
	$(OBJCOPY) --set-section-flags=image=alloc,load,code ../images/$@.$(BNAME)
	$(OBJCOPY) -O binary --adjust-vma 0x3ed00000 \
	../images/$@.$(BNAME) ../images/$@.$(BNAME).bin

