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
extern struct cfdriver localbus_cd;
extern struct cfdriver inphy_cd;
extern struct cfdriver pci_cd;
extern struct cfdriver fxp_cd;
extern struct cfdriver rtl_cd;
extern struct cfdriver pciide_cd;
extern struct cfdriver wd_cd;

extern struct cfattach mainbus_ca;
extern struct cfattach pcibr_ca;
extern struct cfattach localbus_ca;
extern struct cfattach inphy_ca;
extern struct cfattach pci_ca;
extern struct cfattach fxp_ca;
extern struct cfattach rtl_ca;
extern struct cfattach pciide_ca;
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
	"phy",
	"bus",
	"dev",
	"function",
	"channel",
	"drive",
};

/* each entry is an index into locnames[]; -1 terminates */
short locnamp[] = {
	-1, 0, -1, 1, -1, 2, 3, -1,
	4, 5, -1,
};

/* size of parent vectors */
int pv_size = 10;

/* parent vectors */
short pv[10] = {
	4, -1, 7, -1, 5, -1, 1, -1, 0, -1,
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
    {&pcibr_ca,		&pcibr_cd,	 0, NORM,     loc,    0, pv+ 8, 0, 0,    0},
/*  2: localbus0 at mainbus0 */
    {&localbus_ca,	&localbus_cd,	 0, NORM,     loc,    0, pv+ 8, 0, 0,    0},
/*  3: inphy* at fxp0 phy -1 */
    {&inphy_ca,		&inphy_cd,	 0, STAR, loc+  1,    0, pv+ 4, 1, 0,    0},
/*  4: pci* at pcibr0 bus -1 */
    {&pci_ca,		&pci_cd,	 0, STAR, loc+  1,    0, pv+ 6, 3, 0,    0},
/*  5: fxp0 at pci* dev -1 function -1 */
    {&fxp_ca,		&fxp_cd,	 0, NORM, loc+  0,    0, pv+ 0, 5, 0,    0},
/*  6: rtl* at pci* dev -1 function -1 */
    {&rtl_ca,		&rtl_cd,	 0, STAR, loc+  0,    0, pv+ 0, 5, 0,    0},
/*  7: pciide* at pci* dev -1 function -1 */
    {&pciide_ca,	&pciide_cd,	 0, STAR, loc+  0,    0, pv+ 0, 5, 0,    0},
/*  8: wd* at pciide* channel -1 drive -1 */
    {&wd_ca,		&wd_cd,		 0, STAR, loc+  0,    0, pv+ 2, 8, 0,    0},
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
