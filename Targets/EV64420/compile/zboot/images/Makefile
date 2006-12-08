OBJCOPY = mips-elf-objcopy
pmon.bin.gz: ../../EV64420EL/pmon 
	$(OBJCOPY) -S -O binary ../../EV64420EL/pmon pmon.bin 
	gzip -v9f pmon.bin 

clean:
	rm -f pmon.*  zpmon* 

