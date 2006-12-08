OBJCOPY = mips-elf-objcopy
pmon.bin.gz: ../../IT8172EL/pmon 
	$(OBJCOPY) -S -O binary ../../IT8172EL/pmon pmon.bin 
	gzip -v9f pmon.bin 

clean:
	rm -f pmon.*  zpmon* 

