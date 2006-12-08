images/pmon.bin.gz: ../Bonito/pmon
	$(MAKE) -C images pmon.bin.gz

BOOT_TARGETS = zpmon

$(BOOT_TARGETS): images/pmon.bin.gz
	$(MAKE) -C pmon

# Do the dirs
clean:
	rm -f *.o
	$(MAKE) -C images clean
	$(MAKE) -C pmon clean

