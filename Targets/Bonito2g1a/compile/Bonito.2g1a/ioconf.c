/*
 * MACHINE GENERATED: DO NOT EDIT
 *
 * ioconf.c, from "Bonito.2g1a"
 */

#include "mainbus.h"
#if NMAINBUS > 0
#include <sys/param.h>
#include <sys/device.h>

extern struct cfdriver mainbus_cd;
extern struct cfdriver pcibr_cd;
extern struct cfdriver usb_cd;
extern struct cfdriver localbus_cd;
extern struct cfdriver syn_cd;
extern struct cfdriver pci_cd;
extern struct cfdriver rtl_cd;
extern struct cfdriver pciide_cd;
extern struct cfdriver ppb_cd;
extern struct cfdriver uhci_cd;
extern struct cfdriver ohci_cd;
extern struct cfdriver wd_cd;
extern struct cfdriver ide_cd_cd;

extern struct cfattach mainbus_ca;
extern struct cfattach pcibr_ca;
extern struct cfattach usb_ca;
extern struct cfattach localbus_ca;
extern struct cfattach syn_ca;
extern struct cfattach pci_ca;
extern struct cfattach rtl_ca;
extern struct cfattach pciide_ca;
extern struct cfattach ppb_ca;
extern struct cfattach uhci_ca;
extern struct cfattach ohci_ca;
extern struct cfattach wd_ca;
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
	"base",
	"bus",
	"dev",
	"function",
	"channel",
	"drive",
};

/* each entry is an index into locnames[]; -1 terminates */
short locnamp[] = {
	-1, 0, -1, 1, -1, 1, -1, 2,
	3, -1, 4, 5, -1,
};

/* size of parent vectors */
int pv_size = 14;

/* parent vectors */
short pv[14] = {
	1, 9, -1, 11, 10, -1, 6, -1, 8, -1, 3, -1, 0, -1,
};

#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR
#define DNRM FSTATE_DNOTFOUND
#define DSTR FSTATE_DSTAR

struct cfdata cfdata[] = {
    /* attachment       driver        unit  state loc     flags parents nm ivstubs starunit1 */
/*  0: mainbus0 at root */
    {&mainbus_ca,	&mainbus_cd,	 0, NORM,     loc,    0, pv+ 2, 0, 0,    0},
/*  1: pcibr0 at mainbus0 */
    {&pcibr_ca,		&pcibr_cd,	 0, NORM,     loc,    0, pv+12, 0, 0,    0},
/*  2: usb* at ohci*|uhci* */
    {&usb_ca,		&usb_cd,	 0, STAR,     loc,    0, pv+ 3, 0, 0,    0},
/*  3: localbus0 at mainbus0 */
    {&localbus_ca,	&localbus_cd,	 0, NORM,     loc,    0, pv+12, 0, 0,    0},
/*  4: syn0 at localbus0 base -1 */
    {&syn_ca,		&syn_cd,	 0, NORM, loc+  1,    0, pv+10, 1, 0,    0},
/*  5: syn1 at localbus0 base -1 */
    {&syn_ca,		&syn_cd,	 1, NORM, loc+  1,    0, pv+10, 1, 0,    1},
/*  6: pci* at pcibr0|ppb* bus -1 */
    {&pci_ca,		&pci_cd,	 0, STAR, loc+  1,    0, pv+ 0, 3, 0,    0},
/*  7: rtl* at pci* dev -1 function -1 */
    {&rtl_ca,		&rtl_cd,	 0, STAR, loc+  0,    0, pv+ 6, 7, 0,    0},
/*  8: pciide* at pci* dev -1 function -1 */
    {&pciide_ca,	&pciide_cd,	 0, STAR, loc+  0,    0, pv+ 6, 7, 0,    0},
/*  9: ppb* at pci* dev -1 function -1 */
    {&ppb_ca,		&ppb_cd,	 0, STAR, loc+  0,    0, pv+ 6, 7, 0,    0},
/* 10: uhci* at pci* dev -1 function -1 */
    {&uhci_ca,		&uhci_cd,	 0, STAR, loc+  0,    0, pv+ 6, 7, 0,    0},
/* 11: ohci* at pci* dev -1 function -1 */
    {&ohci_ca,		&ohci_cd,	 0, STAR, loc+  0,    0, pv+ 6, 7, 0,    0},
/* 12: wd0 at pciide* channel -1 drive -1 */
    {&wd_ca,		&wd_cd,		 0, NORM, loc+  0,    0, pv+ 8, 10, 0,    0},
/* 13: ide_cd* at pciide* channel -1 drive -1 */
    {&ide_cd_ca,	&ide_cd_cd,	 0, STAR, loc+  0,  0x1, pv+ 8, 10, 0,    0},
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
