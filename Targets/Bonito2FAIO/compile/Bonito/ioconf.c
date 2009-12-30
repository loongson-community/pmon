/*
 * MACHINE GENERATED: DO NOT EDIT
 *
 * ioconf.c, from "Bonito"
 */

#include "mainbus.h"
#if NMAINBUS > 0
#include <sys/param.h>
#include <sys/device.h>

extern struct cfdriver mainbus_cd;
extern struct cfdriver pcibr_cd;
extern struct cfdriver usb_cd;
extern struct cfdriver sata_cd;
extern struct cfdriver localbus_cd;
extern struct cfdriver pci_cd;
extern struct cfdriver rtl_cd;
extern struct cfdriver pciide_cd;
extern struct cfdriver ohci_cd;
extern struct cfdriver rtk_cd;
extern struct cfdriver wd_cd;
extern struct cfdriver vt_cd;
extern struct cfdriver ide_cd_cd;

extern struct cfattach mainbus_ca;
extern struct cfattach pcibr_ca;
extern struct cfattach usb_ca;
extern struct cfattach sata_ca;
extern struct cfattach localbus_ca;
extern struct cfattach pci_ca;
extern struct cfattach rtl_ca;
extern struct cfattach pciide_ca;
extern struct cfattach ohci_ca;
extern struct cfattach rtk_ca;
extern struct cfattach wd_ca;
extern struct cfattach vt_ca;
extern struct cfattach ide_cd_ca;


/* locators */
static int loc[2] = {
	-1, -1,
};

#ifndef MAXEXTRALOC
#define MAXEXTRALOC 32
#endif
int extraloc[MAXEXTRALOC];
int nextraloc = MAXEXTRALOC;
int uextraloc = 0;

char *locnames[] = {
	"bus",
	"dev",
	"function",
	"channel",
	"drive",
};

/* each entry is an index into locnames[]; -1 terminates */
short locnamp[] = {
	-1, 0, -1, 1, 2, -1, 3, 4,
	-1,
};

/* size of parent vectors */
int pv_size = 12;

/* parent vectors */
short pv[12] = {
	5, -1, 7, -1, 1, -1, 0, -1, 8, -1, 11, -1,
};

#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR
#define DNRM FSTATE_DNOTFOUND
#define DSTR FSTATE_DSTAR

struct cfdata cfdata[] = {
    /* attachment       driver        unit  state loc     flags parents nm ivstubs starunit1 */
/*  0: mainbus0 at root */
    {&mainbus_ca,	&mainbus_cd,	 0, NORM,     loc,    0, pv+ 1, 0, 0,    0},
/*  1: pcibr0 at mainbus0 */
    {&pcibr_ca,		&pcibr_cd,	 0, NORM,     loc,    0, pv+ 6, 0, 0,    0},
/*  2: usb* at ohci0 */
    {&usb_ca,		&usb_cd,	 0, STAR,     loc,    0, pv+ 8, 0, 0,    0},
/*  3: sata* at vt0 */
    {&sata_ca,		&sata_cd,	 0, STAR,     loc,    0, pv+10, 0, 0,    0},
/*  4: localbus0 at mainbus0 */
    {&localbus_ca,	&localbus_cd,	 0, NORM,     loc,    0, pv+ 6, 0, 0,    0},
/*  5: pci* at pcibr0 bus -1 */
    {&pci_ca,		&pci_cd,	 0, STAR, loc+  1,    0, pv+ 4, 1, 0,    0},
/*  6: rtl0 at pci* dev -1 function -1 */
    {&rtl_ca,		&rtl_cd,	 0, NORM, loc+  0,    0, pv+ 0, 3, 0,    0},
/*  7: pciide* at pci* dev -1 function -1 */
    {&pciide_ca,	&pciide_cd,	 0, STAR, loc+  0,    0, pv+ 0, 3, 0,    0},
/*  8: ohci0 at pci* dev -1 function -1 */
    {&ohci_ca,		&ohci_cd,	 0, NORM, loc+  0,    0, pv+ 0, 3, 0,    0},
/*  9: rtk0 at pci* dev -1 function -1 */
    {&rtk_ca,		&rtk_cd,	 0, NORM, loc+  0,    0, pv+ 0, 3, 0,    0},
/* 10: wd* at pciide* channel -1 drive -1 */
    {&wd_ca,		&wd_cd,		 0, STAR, loc+  0,    0, pv+ 2, 6, 0,    0},
/* 11: vt0 at pci* dev -1 function -1 */
    {&vt_ca,		&vt_cd,		 0, NORM, loc+  0,    0, pv+ 0, 3, 0,    0},
/* 12: ide_cd* at pciide* channel -1 drive -1 */
    {&ide_cd_ca,	&ide_cd_cd,	 0, STAR, loc+  0,  0x1, pv+ 2, 6, 0,    0},
    {0},
    {0},
    {0},
    {0},
    {0},
    {0},
    {0},
    {0},
    {(struct cfattach *)-1}
};

short cfroots[] = {
	 0 /* mainbus0 */,
	-1
};

int cfroots_size = 2;

/* pseudo-devices */
extern void loopattach (int);

char *pdevnames[] = {
	"loop",
};

int pdevnames_size = 1;

struct pdevinit pdevinit[] = {
	{ loopattach, 1 },
	{ 0, 0 }
};
#endif /* NMAINBUS */
