OBJCOPY = mips-elf-objcopy
pmon.bin.gz: ../../Bonito/pmon 
	$(OBJCOPY) -S -O binary ../../Bonito/pmon pmon.bin 
	gzip -v9f pmon.bin 

clean:
	rm -f pmon.*  zpmon* 

