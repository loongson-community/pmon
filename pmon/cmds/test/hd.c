#include <sys/device.h>
#include <sys/disk.h>
#include <sys/buf.h>
#include <dev/ata/wdvar.h>
#include <dev/ata/atareg.h>
#include <machine/param.h>
struct wd_softc {
	/* General disk infos */
	struct device sc_dev;
	struct disk sc_dk;
	struct buf sc_q;
	/* IDE disk soft states */
	struct ata_bio sc_wdc_bio; /* current transfer */
	struct buf *sc_bp; /* buf being transfered */
	struct ata_drive_datas *drvp; /* Our controller's infos */
	int openings;
	struct ataparams sc_params;/* drive characteistics found */
	int sc_flags;	  
#define WDF_LOCKED	  0x01
#define WDF_WANTED	  0x02
#define WDF_WLABEL	  0x04 /* label is writable */
#define WDF_LABELLING   0x08 /* writing label */
/*
 * XXX Nothing resets this yet, but disk change sensing will when ATA-4 is
 * more fully implemented.
 */
#define WDF_LOADED	  0x10 /* parameters loaded */
#define WDF_WAIT	0x20 /* waiting for resources */
#define WDF_LBA	 0x40 /* using LBA mode */

	int sc_capacity;
	int cyl; /* actual drive parameters */
	int heads;
	int sectors;
	int retries; /* number of xfer retry */
#if NRND > 0
	rndsource_element_t	rnd_source;
#endif
	void *sc_sdhook;
};
extern struct cfdriver wd_cd;
#define wdlookup(unit) (struct wd_softc *)device_lookup(&wd_cd, (unit))
#define device_unref(x)
extern void (*__msgbox)(int yy,int xx,int height,int width,char *msg);

int hdtest()
{
struct wd_softc *wd;
char str[20];
FILE *fp;
char fname[0x40];
unsigned char buf[512];
unsigned char buf1[512];
int i,j;
int found=0;
int errors;
printf("begin harddisk test\n");
printf("get harddisk info:\n");
//---register test--
for(i=0;i<4;i++)
 {
	wd = wdlookup(i);
	if(!wd)continue;
	found++;
	printf("hd%d ",found);
//-----get hd info---	
	if ((wd->sc_flags & WDF_LBA) != 0) {
		wd->sc_capacity =
		    (wd->sc_params.atap_capacity[1] << 16) |
		    wd->sc_params.atap_capacity[0];
		printf(" LBA, %dMB, %d cyl, %d head, %d sec, %d sectors\n",
		    wd->sc_capacity / (1048576 / DEV_BSIZE),
		    wd->sc_params.atap_cylinders,
		    wd->sc_params.atap_heads,
		    wd->sc_params.atap_sectors,
		    wd->sc_capacity);
	} else {
		wd->sc_capacity =
		    wd->sc_params.atap_cylinders *
		    wd->sc_params.atap_heads *
		    wd->sc_params.atap_sectors;
		printf(" CHS, %dMB, %d cyl, %d head, %d sec, %d sectors\n",
		    wd->sc_capacity / (1048576 / DEV_BSIZE),
		    wd->sc_params.atap_cylinders,
		    wd->sc_params.atap_heads,
		    wd->sc_params.atap_sectors,
		    wd->sc_capacity);
	}
	device_unref(wd);    
//-----read write test-----
printf("start harddisk read write test\n");
	sprintf(fname,"/dev/disk/wd%d",i);
	fp=fopen(fname,"r+");
	fseek(fp,1024,SEEK_SET);
	fread(buf,512,1,fp);
	fclose(fp);

	memset(buf1,0x5a,512);
	fp=fopen(fname,"r+");
	fseek(fp,1024,SEEK_SET);
	fwrite(buf1,512,1,fp);
	fclose(fp);

	fp=fopen(fname,"r+");
	fseek(fp,1024,SEEK_SET);
	fread(buf1,512,1,fp);
	fclose(fp);

	fp=fopen(fname,"r+");
	fseek(fp,1024,SEEK_SET);
	fwrite(buf,512,1,fp);
	fclose(fp);
	
	errors=0;
	for(j=0;j<512;j++)
	if(buf1[j]!=0x5a)
	{
		printf("read write test error,write 0x5a read %x\n",buf1[j]);
		errors++;
	}
	if(!errors){
	   printf("harddisk read write test ok\n");
//	   printf("press <Enter> to continue !!\n");
	}

 }
if(!found){ 
       printf("can not found harddisk\n");
//	   printf("press <Enter> to continue !!\n");
//	   return 0;
       return 1;
	 }
#if 0	 
	gets(str);
#endif
return 0;
}
