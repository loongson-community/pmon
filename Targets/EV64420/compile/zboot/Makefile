images/pmon.bin.gz: ../EV64420EL/pmon
	$(MAKE) -C images pmon.bin.gz

BOOT_TARGETS = zpmon

$(BOOT_TARGETS): images/pmon.bin.gz
	$(MAKE) -C pmon

# Do the dirs
clean:
	$(MAKE) -C images clean
	$(MAKE) -C pmon clean

