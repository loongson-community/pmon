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
extern struct cfdriver loopdev_cd;
extern struct cfdriver localbus_cd;
extern struct cfdriver pci_cd;
extern struct cfdriver rte_cd;
extern struct cfdriver pciide_cd;
extern struct cfdriver ppb_cd;
extern struct cfdriver ohci_cd;
extern struct cfdriver wd_cd;

extern struct cfattach mainbus_ca;
extern struct cfattach pcibr_ca;
extern struct cfattach usb_ca;
extern struct cfattach loopdev_ca;
extern struct cfattach localbus_ca;
extern struct cfattach pci_ca;
extern struct cfattach rte_ca;
extern struct cfattach pciide_ca;
extern struct cfattach ppb_ca;
extern struct cfattach ohci_ca;
extern struct cfattach wd_ca;


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
	-1, 0, -1, 0, -1, 1, 2, -1,
	3, 4, -1,
};

/* size of parent vectors */
int pv_size = 11;

/* parent vectors */
short pv[11] = {
	1, 8, -1, 5, -1, 7, -1, 0, -1, 9, -1,
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
    {&pcibr_ca,		&pcibr_cd,	 0, NORM,     loc,    0, pv+ 7, 0, 0,    0},
/*  2: usb* at ohci* */
    {&usb_ca,		&usb_cd,	 0, STAR,     loc,    0, pv+ 9, 0, 0,    0},
/*  3: loopdev0 at mainbus0 */
    {&loopdev_ca,	&loopdev_cd,	 0, NORM,     loc,    0, pv+ 7, 0, 0,    0},
/*  4: localbus0 at mainbus0 */
    {&localbus_ca,	&localbus_cd,	 0, NORM,     loc,    0, pv+ 7, 0, 0,    0},
/*  5: pci* at pcibr0|ppb* bus -1 */
    {&pci_ca,		&pci_cd,	 0, STAR, loc+  1,    0, pv+ 0, 1, 0,    0},
/*  6: rte* at pci* dev -1 function -1 */
    {&rte_ca,		&rte_cd,	 0, STAR, loc+  0,    0, pv+ 3, 5, 0,    0},
/*  7: pciide* at pci* dev -1 function -1 */
    {&pciide_ca,	&pciide_cd,	 0, STAR, loc+  0,    0, pv+ 3, 5, 0,    0},
/*  8: ppb* at pci* dev -1 function -1 */
    {&ppb_ca,		&ppb_cd,	 0, STAR, loc+  0,    0, pv+ 3, 5, 0,    0},
/*  9: ohci* at pci* dev -1 function -1 */
    {&ohci_ca,		&ohci_cd,	 0, STAR, loc+  0,    0, pv+ 3, 5, 0,    0},
/* 10: wd* at pciide* channel -1 drive -1 */
    {&wd_ca,		&wd_cd,		 0, STAR, loc+  0,    0, pv+ 5, 8, 0,    0},
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
