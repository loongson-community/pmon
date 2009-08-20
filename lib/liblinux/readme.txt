This lib is for replant linux drivers to pmon.
1)get source and makefile form linux
cd linux-2.6.18
linux2pmon drivers/scsi/libata-scsi.o
then will generate /tmp/1.tar and /tmp/1.Make.
cd pmon-all.64/lib/liblinux
tar -xvf /tmp/1.tar
cat /tmp/1.Make >> Makefile.rules
2)edit Makefile
append drivers/scsi/libata-scsi.o  to OBJS.
This lib is for replant linux drivers to pmon.
1)get source and makefile form linux
cd linux-2.6.18
linux2pmon drivers/scsi/libata-scsi.o
then will generate /tmp/1.tar and /tmp/1.Make.
cd pmon-all.64/lib/liblinux
tar -xvf /tmp/1.tar
cat /tmp/1.Make >> Makefile.rules
2)edit Makefile
append drivers/scsi/libata-scsi.o  to OBJS.
