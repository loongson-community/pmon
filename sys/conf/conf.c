/*	$OpenBSD: conf.c,v 1.12 2011/01/14 19:04:9 jasper Exp $ */

/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/scsi/sd.h>


/*
 *	Block devices.
 */

#include "sd.h"
#include "cd.h"
#include "wd.h"
bdev_decl(wd);

struct bdevsw	bdevsw[] =
{
	bdev_disk_init(NSD,sd),		/* 0: SCSI disk */
	bdev_notdef(),			/* 1:  */
	bdev_notdef(),			/* 2:  */
	bdev_notdef(),			/* 3:  */
	bdev_notdef(),			/* 4:  */
	bdev_notdef(),			/* 5:  */
	bdev_notdef(),			/* 6:  */
	bdev_notdef(),			/* 7:  */
	bdev_notdef(),			/* 8:  */
	bdev_notdef(),			/* 9:  */
	bdev_notdef(),			/* 10:  */
	bdev_notdef(),			/* 11:  */
	bdev_notdef(),			/* 12:  */
	bdev_notdef(),			/* 13:  */
	bdev_notdef(),			/* 14:  */
	bdev_notdef(),			/* 15:  */
};

int	nblkdev = nitems(bdevsw);

/*
 *	Character devices.
 */

#define mmread mmrw
#define mmwrite mmrw
dev_type_read(mmrw);
cdev_decl(mm);
cdev_decl(wd);
#ifdef NNPFS
#include <nnpfs/nnnpfs.h>
cdev_decl(nnpfs_dev);
#endif
cdev_decl(pci);

struct cdevsw	cdevsw[] =
{
	cdev_notdef(),			/* 0: */
	cdev_notdef(),			/* 1: */
	cdev_notdef(),			/* 2: */
	cdev_notdef(),			/* 3: */
	cdev_notdef(),			/* 4: */
	cdev_notdef(),			/* 5: */
	cdev_notdef(),			/* 6: */
	cdev_notdef(),			/* 7: */
	cdev_notdef(),			/* 8: */
	cdev_disk_init(NSD,sd),		/* 9: SCSI disk */
	cdev_notdef(),			/* 15: */
	cdev_notdef(),			/* 19: */
	cdev_notdef(),			/* 20: */
	cdev_notdef(),			/* 21: */
	cdev_notdef(),			/* 24: */
#ifdef USER_PCICONF
	cdev_pci_init(NPCI,pci),	/* 29: PCI user */
#else
	cdev_notdef(),			/* 29 */
#endif
	cdev_notdef(),			/* 30: */
	cdev_notdef(),			/* 34: */
	cdev_notdef(),			/* 37: */
	cdev_notdef(),			/* 38: */
	cdev_notdef(),			/* 39: */
	cdev_notdef(),			/* 40: */
	cdev_notdef(),			/* 41: */
	cdev_notdef(),			/* 42: */
	cdev_notdef(),			/* 33: */
	cdev_notdef(),			/* 46: */
	cdev_notdef(),			/* 48: */
#ifdef NNPFS
	cdev_nnpfs_init(NNNPFS,nnpfs_dev),	/* 51: nnpfs communication device */
#else
	cdev_notdef(),			/* 51: */
#endif
	cdev_notdef(),			/* 53: */
	cdev_notdef(),			/* 54: */
	cdev_notdef(),			/* 55: */
	cdev_notdef(),			/* 56: */
	cdev_notdef(),			/* 57: */
	cdev_notdef(),			/* 58: */
	cdev_notdef(),			/* 59: */
	cdev_notdef(),			/* 60: */
};

int	nchrdev = nitems(cdevsw);

/*
 * Swapdev is a fake device implemented
 * in sw.c used only internally to get to swstrategy.
 * It cannot be provided to the users, because the
 * swstrategy routine munches the b_dev and b_blkno entries
 * before calling the appropriate driver.  This would horribly
 * confuse, e.g. the hashing routines. Instead, /dev/drum is
 * provided as a character (raw) device.
 */
dev_t	swapdev = makedev(1, 0);

/*
 * Routine that identifies /dev/mem and /dev/kmem.
 *
 * A minimal stub routine can always return 0.
 */
int
iskmemdev( dev_t dev)
{

	if (major(dev) == 3 && (minor(dev) == 0 || minor(dev) == 1))
		return (1);
	return (0);
}

/*
 * Returns true if def is /dev/zero
 */
int
iszerodev( dev_t dev )
{
	return (major(dev) == 3 && minor(dev) == 12);
}
dev_t
getnulldev(void)
{
	return(makedev(3, 2));
}


int chrtoblktbl[] =  {
	/* VCHR         VBLK */
	/* 0 */		NODEV,
	/* 1 */		NODEV,
	/* 2 */		NODEV,
	/* 3 */		NODEV,
	/* 4 */		NODEV,
	/* 5 */		NODEV,
	/* 6 */		NODEV,
	/* 7 */		NODEV,
	/* 8 */		3,		/* cd */
	/* 9 */		0,		/* sd */
	/* 10 */	10,		/* st */
	/* 11 */	2,		/* vnd */
	/* 12 */	NODEV,
	/* 13 */	NODEV,
	/* 14 */	NODEV,
	/* 15 */	NODEV,
	/* 16 */	NODEV,
	/* 17 */	NODEV,
	/* 18 */	4,		/* wd */
	/* 19 */	NODEV,
	/* 20 */	NODEV,
	/* 21 */	NODEV,
	/* 22 */	8,		/* rd */
	/* 23 */	6		/* ccd */
};

extern int chrtoblktbl[];

int nchrtoblktbl = nitems(chrtoblktbl);

extern int nchrtoblktbl;
#include <dev/cons.h>

cons_decl(ws);
cons_decl(com);

struct	consdev constab[] = {
#if NWSDISPLAY > 0
	cons_init(ws),
#endif
#if NCOM > 0
	cons_init(com),
#endif
	{ 0 },
};

/*
 * Unsupported device function (e.g. writing to read-only device).
 */
int
enodev(void)
{

	return (ENODEV);
}
