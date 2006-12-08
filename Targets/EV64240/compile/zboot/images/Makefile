OBJCOPY = mips-elf-objcopy
pmon.bin.gz: ../../EV64240EL/pmon 
	$(OBJCOPY) -S -O binary ../../EV64240EL/pmon pmon.bin 
	gzip -v9f pmon.bin 

clean:
	rm -f pmon.*  zpmon* 

