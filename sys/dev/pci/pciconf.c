/*	$Id: pciconf.c,v 1.1.1.1 2006/09/14 01:59:08 root Exp $ */

/*
 * Copyright (c) 2000 Opsycon AB  (www.opsycon.se)
 * Copyright (c) 2000 Rtmx, Inc   (www.rtmx.com)
 * Copyright (c) 2001 IP Unplugged AB  (www.ipunplugged.com)
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
 *	This product includes software developed for Rtmx, Inc by
 *	Opsycon Open System Consulting AB, Sweden.
 *      This product includes software developed for IP Inplugged AB, by
 *      Patrik Lindergren.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 *  This work is derivate work created from code written at Algorithmics UK.
 *  It was part of PMON which is not copyrighted. After modifications and
 *  extensions it is now released under standard BSD copyright.
 */

/*
 * pciconf.c: generic PCI bus configuration
 */

#include <sys/param.h>
#include <stdarg.h>
#include <progress.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <machine/bus.h>
#include <include/pmon_target.h>
#include <stdbool.h>

#include "pcivar.h"
#include "pcireg.h"

#if defined(LOONGSON_2K) || defined(LS7A)
#define PCIVERBOSE 5
#else
#define PCIVERBOSE 0
#endif
#ifdef PCIVERBOSE
#include "pcidevs.h"
#endif


#include <sys/systm.h>
#include <pmon.h>
extern char *getenv __P((const char *));
extern long atol __P((const char *));

extern void *pmalloc __P((size_t ));
extern void pfree __P((void * ));

static int _pci_roundup(int , int );
static int _pci_getIntRouting(struct pci_device *);
static int _pci_setupIntRouting(struct pci_device *);
static void _pci_scan_dev(struct pci_device *dev, int bus, int device, int initialise);
static void _insertsort_window(struct pci_win **, struct pci_win *);
static void _pci_device_insert(struct pci_device *parent, struct pci_device *device);
pcireg_t _pci_allocate_mem __P((struct pci_device *, vm_size_t, unsigned int));
pcireg_t _pci_allocate_io __P((struct pci_device *, vm_size_t, unsigned int));
static void _setup_pcibuses(int );
static void _pci_bus_insert(struct pci_bus *);

#ifdef PCIE_GRAPHIC_CARD
bool is_pcie_vga_card();
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#define PRINTF printf
#define VPRINTF vprintf

#ifndef PCIVERBOSE
#define _PCIVERBOSE 0
#else
#define _PCIVERBOSE PCIVERBOSE
#endif


pcitag_t have_vga = 0;			/* Have tag if VGA board found */
int monarch_mode = 1;			/* Default as master on the bus! */
int pci_roots;				/* How many pci roots to init */
int _pciverbose = _PCIVERBOSE;
static int _pci_nbus = 0;		/* Allow for eight roots */

struct tgt_bus_space def_bus_iot;		/* Default bus tags */
struct tgt_bus_space def_bus_memt;		/* Default bus tags */
struct pci_device *_pci_head;
struct pci_bus *_pci_bushead;
#ifdef LOONGSON_3A2H
struct pci_device *_pci_head1;
struct pci_device *_pci_head2;
struct pci_device *_pci_head3;
struct pci_bus *_pci_bushead1;
struct pci_bus *_pci_bushead2;
struct pci_bus *_pci_bushead3;
#endif
struct pci_intline_routing *_pci_inthead;
struct pci_device *vga_dev = NULL,*pcie_dev = NULL;

static inline int ffs(int x)
{
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}

bool is_power_of_2(unsigned long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

/* read max payload size support */
static int pcie_get_mpss(struct pci_device *dev)
{
	pcitag_t tag;
	int offset;
	pcireg_t value;

	tag = _pci_make_tag(dev->pa.pa_bus, dev->pa.pa_device, dev->pa.pa_function);
	if (pci_get_capability(0, tag, PCI_CAP_ID_EXP, &offset, &value)) {
		value = _pci_conf_read(tag, offset + PCI_EXP_DEVCAP);
		return 128 << (value & PCI_EXP_DEVCAP_PAYLOAD);
	}

	return 128;
}

/* read max payload size */
static int pcie_get_mps(struct pci_device *dev)
{
	pcitag_t tag;
	int offset;
	pcireg_t value;

	tag = _pci_make_tag(dev->pa.pa_bus, dev->pa.pa_device, dev->pa.pa_function);
	if (pci_get_capability(0, tag, PCI_CAP_ID_EXP, &offset, &value)) {
		value = _pci_conf_read(tag, offset + PCI_EXP_DEVCTL);
		return 128 << ((value & PCI_EXP_DEVCTL_PAYLOAD) >> 5);
	}

	return 128;
}

static int pcie_set_mps(struct pci_device *dev, int mps)
{
	unsigned int v;
	pcitag_t tag;
	int offset;
	pcireg_t value;
	pcireg_t devctl;

	if (mps < 128 || mps > 4096 || !is_power_of_2(mps))
		return -EINVAL;

	tag = _pci_make_tag(dev->pa.pa_bus, dev->pa.pa_device, dev->pa.pa_function);
	if (pci_get_capability(0, tag, PCI_CAP_ID_EXP, &offset, &value)) {
		v = ffs(mps) - 8;
		v <<= 5;

		devctl = _pci_conf_read(tag, offset + PCI_EXP_DEVCTL);
		devctl &= ~PCI_EXP_DEVCTL_PAYLOAD;
		devctl |= v;
		_pci_conf_write(dev->pa.pa_tag, offset + PCI_EXP_DEVCTL, devctl);
	}

	return 0;
}

/* read max read request size */
static int pcie_get_readrq(struct pci_device *dev)
{
	pcitag_t tag;
	int offset;
	pcireg_t value;

	tag = _pci_make_tag(dev->pa.pa_bus, dev->pa.pa_device, dev->pa.pa_function);
	if (pci_get_capability(0, tag, PCI_CAP_ID_EXP, &offset, &value)) {
		value = _pci_conf_read(tag, offset + PCI_EXP_DEVCTL);
		return 128 << ((value & PCI_EXP_DEVCTL_READRQ) >> 12);
	}

	return 128;
}

static int pcie_set_readrq(struct pci_device *dev, int rq)
{
	unsigned int v;
	int mps;
	pcitag_t tag;
	int offset;
	pcireg_t value;
	pcireg_t devctl;

	if (rq < 128 || rq > 4096 || !is_power_of_2(rq))
		return -EINVAL;

	tag = _pci_make_tag(dev->pa.pa_bus, dev->pa.pa_device, dev->pa.pa_function);
	/*
	 * If using the "performance" PCIe config, we clamp the
	 * read rq size to the max packet size to prevent the
	 * host bridge generating requests larger than we can
	 * cope with
	 */
	mps = pcie_get_mps(dev);

	if (mps < rq)
		rq = mps;

	if (pci_get_capability(0, tag, PCI_CAP_ID_EXP, &offset, &value)) {
		v = (ffs(rq) - 8) << 12;
		devctl = _pci_conf_read(tag, offset + PCI_EXP_DEVCTL);
		devctl &= ~PCI_EXP_DEVCTL_READRQ;
		devctl |= v;
		_pci_conf_write(tag, offset + PCI_EXP_DEVCTL, devctl);
	}

	return 0;
}

/* modify max payload size as minimal value of bridge and this device */
static void pcie_write_mps(struct pci_device *dev)
{
	int rc;
	struct pci_device *pcidev;
	int value;
	int mps;

	mps = 4096;

	/* invalid device */
	if (PCI_VENDOR(_pci_conf_read(dev->pa.pa_tag, PCI_ID_REG)) == 0xffff)
		return;

	/* find the minimal max payload size */
	for (pcidev = dev; pcidev != NULL; pcidev = pcidev->parent) {
		/* 2k1000 skip bus 0, device 0, function 0 */
		if (pcidev->pa.pa_tag == 0)
			continue;

		value = pcie_get_mpss(pcidev);

		if (mps > value)
			mps = value;
	}

	for (pcidev = dev; pcidev != NULL; pcidev = pcidev->parent) {

		/* 2k1000 skip bus 0, device 0, function 0 */
		if (pcidev->pa.pa_tag == 0)
			continue;

		rc = pcie_set_mps(pcidev, mps);
		if (rc)
			printf("Failed attempting to set the MPS\n");
	}
}

/* modify max request read size */
static void pcie_write_mrrs(struct pci_device *dev)
{
	int rc, mrrs;
	struct pci_device *pcidev;

	/* For Max performance, the MRRS must be set to the largest supported
	 * value.  However, it cannot be configured larger than the MPS the
	 * device or the bus can support.  This should already be properly
	 * configured by a prior call to pcie_write_mps.
	 */
	mrrs = pcie_get_mps(dev);

	/* MRRS is a R/W register.  Invalid values can be written, but a
	 * subsequent read will verify if the value is acceptable or not.
	 * If the MRRS value provided is not acceptable (e.g., too large),
	 * shrink the value until it is acceptable to the HW.
	 */
	for (pcidev = dev; pcidev != NULL; pcidev = pcidev->parent) {

		/* 2k1000 skip bus 0, device 0, function 0 */
		if (pcidev->pa.pa_tag == 0)
			continue;

		rc = pcie_set_readrq(pcidev, mrrs);
		if (rc)
			printf("Failed attempting to set the MPS\n");
	}
}

static void
print_bdf (int bus, int device, int function)
{
    PRINTF ("PCI");
    PRINTF (" bus %d", bus);
    if (device >= 0)
	PRINTF (" slot %d", device);
    if (function >= 0)
	PRINTF ("/%d", function);
    PRINTF (": ");
}

void
_pci_bdfprintf (int bus, int device, int function, const char *fmt, ...)
{
    va_list arg;

    print_bdf (bus, device, function);
    va_start(arg, fmt);
    VPRINTF (fmt, arg);
    va_end(arg);
}

void
_pci_tagprintf (pcitag_t tag, const char *fmt, ...)
{
    va_list arg;
    int bus, device, function;

    _pci_break_tag (tag, &bus, &device, &function); 
    print_bdf (bus, device, function);

    va_start(arg, fmt);
    VPRINTF (fmt, arg);
    va_end(arg);
}

static inline int fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

/*
 * Scan each PCI device on the system and record its configuration
 * requirements.
 */

static void
_pci_query_dev_func (struct pci_device *dev, pcitag_t tag, int initialise)
{
    pcireg_t id, class;
    pcireg_t old, mask;
    pcireg_t stat;
    pcireg_t bparam;
    int reg;
    struct pci_bus *pb;
    struct pci_device *pd;
    unsigned int x;
    int bus, device, function;
    int isbridge = 0;
    
    class = _pci_conf_read(tag, PCI_CLASS_REG);
    id = _pci_conf_read(tag, PCI_ID_REG);

    if (_pciverbose) {
        int supported;
        char devinfo[256];
        _pci_devinfo(id, class, &supported, devinfo);
        _pci_tagprintf (tag, "%s\n", devinfo);
    }

    pd = pmalloc(sizeof(struct pci_device));
    if(pd == NULL) {
        PRINTF ("pci: can't alloc memory for device\n");
        return;
    }

    _pci_break_tag (tag, &bus, &device, &function);

    pd->pa.pa_bus = bus;
    pd->pa.pa_device = device;
    pd->pa.pa_function = function;
    pd->pa.pa_tag = tag;
    pd->pa.pa_id = id;
    pd->pa.pa_class = class;
    pd->pa.pa_flags = PCI_FLAGS_IO_ENABLED | PCI_FLAGS_MEM_ENABLED;
    pd->pa.pa_iot = dev->pa.pa_iot;
    pd->pa.pa_memt = dev->pa.pa_memt;
    pd->pa.pa_dmat = dev->pa.pa_dmat;
    pd->parent = dev;
    pd->pcibus = dev->bridge.secbus;
    pb = pd->pcibus;
    _pci_device_insert(dev, pd);

    /*
     * Calculated Interrupt routing
     */
    _pci_setupIntRouting(pd);

    /*
     *  Shut off device if we initialize from non reset.
     */
    stat = _pci_conf_read(tag, PCI_COMMAND_STATUS_REG);
    stat &= ~(PCI_COMMAND_MASTER_ENABLE |
          PCI_COMMAND_IO_ENABLE |
          PCI_COMMAND_MEM_ENABLE);
#ifdef USE_SM502_UART0
    if(device!=14)
    {
#endif
        _pci_conf_write(tag, PCI_COMMAND_STATUS_REG, stat);
#ifdef USE_SM502_UART0
    }
#endif
    pd->stat = stat;

    /* do all devices support fast back-to-back */
    if ((stat & PCI_STATUS_BACKTOBACK_SUPPORT) == 0) {
        pb->fast_b2b = 0;  /* no, sorry */
    }

    /* do all devices run at 66 MHz */
    if ((stat & PCI_STATUS_66MHZ_SUPPORT) == 0) {
        pb->freq66 = 0;   /* no, sorry */
    }

    /* find slowest devsel */
    x = stat & PCI_STATUS_DEVSEL_MASK;
    if (x > pb->devsel) {
        pb->devsel = x;
    }

    /* Funny looking code which deals with any 32bit read only cfg... */
    bparam = _pci_conf_read(tag, (PCI_MINGNT & ~0x3));
    pd->min_gnt = 0xff & (bparam >> ((PCI_MINGNT & 3) * 8));
    bparam = _pci_conf_read(tag, (PCI_MAXLAT & ~0x3));
    pd->max_lat = 0xff & (bparam >> ((PCI_MAXLAT & 3) * 8));

    if (pd->min_gnt != 0 || pd->max_lat != 0) {
        /* find largest minimum grant time of all devices */
        if (pd->min_gnt != 0 && pd->min_gnt > pb->min_gnt) {
            pb->min_gnt = pd->min_gnt;
        }
    
        /* find smallest maximum latency time of all devices */
        if (pd->max_lat != 0 && pd->max_lat < pb->max_lat) {
                pb->max_lat = pd->max_lat;
        }
    
        /* subtract our min on-bus time per second from bus bandwidth */
        if (initialise) {
            pb->bandwidth -= pd->min_gnt * 4000000 / (pd->min_gnt + pd->max_lat);
        }
    }

    /* Map interrupt to interrupt line (software function only) */
    bparam = _pci_conf_read(tag, PCI_INTERRUPT_REG);
    bparam &= ~(PCI_INTERRUPT_LINE_MASK << PCI_INTERRUPT_LINE_SHIFT);
    bparam |= ((_pci_getIntRouting(pd) & 0xff) << PCI_INTERRUPT_LINE_SHIFT);
    _pci_conf_write(tag, PCI_INTERRUPT_REG, bparam);

    /*
     * Check to see if device is a PCI Bridge
     */
    if (PCI_ISCLASS(class, PCI_CLASS_BRIDGE, PCI_SUBCLASS_BRIDGE_PCI)) {
        struct pci_device *pcidev;
        struct pci_win *pm_mem = NULL;
        struct pci_win *pm_io = NULL;
        struct pci_win *pm;
        pcireg_t tmp;
        isbridge = 1;

        pd->bridge.pribus_num = bus;
        pd->bridge.secbus_num =  ++_pci_nbus;
        /* Set it temperary to same as secondary bus number */
        pd->bridge.subbus_num =  pd->bridge.secbus_num;

        tmp = _pci_conf_read(tag, PCI_PRIBUS_1);
        tmp &= 0xff000000;
        tmp |= pd->bridge.pribus_num;
        tmp |= pd->bridge.secbus_num << 8;
        tmp |= pd->bridge.subbus_num << 16;
        _pci_conf_write(tag, PCI_PRIBUS_1, tmp);

        /* Update sub bus number */
        for(pcidev = dev; pcidev != NULL; pcidev = pcidev->parent) {
            //printf("pcidev = 0x%x\n", pcidev);
            pcidev->bridge.subbus_num = pd->bridge.secbus_num;
            tmp = _pci_conf_read(pcidev->pa.pa_tag, PCI_PRIBUS_1);
            tmp &= 0xff00ffff;
            tmp |= pd->bridge.secbus_num << 16;
            _pci_conf_write(pcidev->pa.pa_tag, PCI_PRIBUS_1, tmp);
        }

        pd->bridge.secbus = pmalloc(sizeof(struct pci_bus));
        if(pd->bridge.secbus == NULL) {
            PRINTF ("pci: can't alloc memory for new pci bus\n");
            return;
        }

        pd->bridge.secbus->max_lat = 255;
        pd->bridge.secbus->fast_b2b = 1;
        pd->bridge.secbus->prefetch = 1;
        pd->bridge.secbus->freq66 = 1;
        pd->bridge.secbus->bandwidth = 4000000;
        pd->bridge.secbus->ndev = 1;
        pd->bridge.secbus->bus = pd->bridge.secbus_num;

        _pci_bus_insert(pd->bridge.secbus);
        {
            extern  struct pci_device *_pci_bus[];
            extern int _max_pci_bus;
            _pci_bus[_max_pci_bus++] = pd;
        }

        /* Scan secondary bus of the bridge */
        _pci_scan_dev(pd, pd->bridge.secbus_num, 0, initialise);

        /*
         * Sum up the address space needed by secondary side of bridge
         */
        if(pm_io == NULL) {
            pm_io = pmalloc(sizeof(struct pci_win));
            if(pm_io == NULL) {
                PRINTF ("pci: can't alloc memory for pci memory window\n");
                return;
            }
            pm_io->device = pd;
            pm_io->reg = PCI_IOBASEL_1;
            pm_io->flags = PCI_MAPREG_TYPE_IO;
        }

        if(pm_mem == NULL) {
            pm_mem = pmalloc(sizeof(struct pci_win));
            if(pm_mem == NULL) {
                PRINTF ("pci: can't alloc memory for pci memory window\n");
                return;
            }

            pm_mem->device = pd;
            pm_mem->reg = PCI_MEMBASE_1;
            pm_mem->flags = PCI_MAPREG_MEM_TYPE_32BIT;
        }

        /* Sum up I/O Space needed */
        {
            int max=0;
            for(pm = pd->bridge.iospace; pm != NULL; pm = pm->next) {
                if(max < pm->align)
                    max = pm->align;
                pm_io->size += pm->size;
            }
            pm_io->size = (pm_io->size + max -1) & ~(max-1);
            if(max < 0x1000) max = 0x1000;
            pd->bridge.io_mask = ~(max-1);
            pm_io->align = max;

        }
        /* Sum up Memory Space needed */
        {
            int max = 0;
            for(pm = pd->bridge.memspace; pm != NULL; pm = pm->next) {
                    if(max < pm->align)
                        max = pm->align;
                pm_mem->size += pm->size;
            }
            pm_mem->size = (pm_mem->size + max -1) & ~(max-1);
            if(max<0x100000) max = 0x100000;
            pd->bridge.mem_mask = ~(max-1);
            pm_mem->align = max;
        }

        if(pm_io) {
            /* Round to minimum granularity requierd for a bridge */
            pm_io->size = _pci_roundup(pm_io->size, 0x1000);
            _insertsort_window(&pd->parent->bridge.iospace, pm_io);
        }
        if(pm_mem) {
            /* Round to minimum granularity requierd for a bridge */
            pm_mem->size = _pci_roundup(pm_mem->size, 0x100000);
            _insertsort_window(&pd->parent->bridge.memspace,pm_mem);
        }
    }
    else if (PCI_ISCLASS(class, PCI_CLASS_MASS_STORAGE, PCI_SUBCLASS_MASS_STORAGE_IDE) &&
        dev->bridge.secbus->minpciioaddr == 0) {
        /*
         * There is no need to setup memory regions for IDE storage devices
         * but only if PCI/ISA I/O space is accessables
         */
        return;

    } 
    //set BAR for this dev
    {
        int skipnext = 0;

#if defined(LOONGSON_2K) || defined(LS7A)
	/* skip bus 0, device 0, function 0 */
	if (pd->pa.pa_tag != 0 && !PCI_ISCLASS(class, PCI_CLASS_BRIDGE, PCI_SUBCLASS_BRIDGE_PCI)) {
		pcie_write_mps(pd);
		pcie_write_mrrs(pd);
	}
#endif

        for (reg = PCI_MAPREG_START; reg < (isbridge ? PCI_MAPREG_PPB_END : PCI_MAPREG_END); reg += 4) {
            struct pci_win *pm;

            if (skipnext) {
                skipnext = 0;
                continue;
            }

            old = _pci_conf_read(tag, reg);
            _pci_conf_write(tag, reg, 0xfffffffe);
            mask = _pci_conf_read(tag, reg);
            _pci_conf_write(tag, reg, old);

            if (mask == 0 || mask == 0xffffffff) {
                continue;
            }

            if (_pciverbose >= 3) {
                _pci_tagprintf (tag, "reg 0x%x = 0x%x\n", reg, mask);
            }

            if (PCI_MAPREG_TYPE(mask) == PCI_MAPREG_TYPE_IO) {
                //mask |= 0xffff0000; /* must be ones */
                pm = pmalloc(sizeof(struct pci_win));
                if(pm == NULL) {
                    PRINTF ("pci: can't alloc memory for pci memory window\n");
                    return;
                }

                pm->device = pd;
                pm->reg = reg;
                pm->flags = PCI_MAPREG_TYPE_IO;
                pm->size = -(PCI_MAPREG_IO_ADDR(mask));
                pm->align = pm->size;
                _insertsort_window(&pd->parent->bridge.iospace, pm);
            }
            else {
                switch (PCI_MAPREG_MEM_TYPE(mask)) {
                case PCI_MAPREG_MEM_TYPE_32BIT:
                case PCI_MAPREG_MEM_TYPE_32BIT_1M:
                    break;
                case PCI_MAPREG_MEM_TYPE_64BIT:
                    _pci_conf_write(tag, reg + 4, 0x0);
                    skipnext = 1;
                    break;
                default:
                    _pci_tagprintf (tag, "reserved mapping type 0x%x\n",
                          PCI_MAPREG_MEM_TYPE(mask));
                    continue;
                }

                if  (!PCI_MAPREG_MEM_PREFETCHABLE(mask)) {
                    pb->prefetch = 0;
                }

                pm = pmalloc(sizeof(struct pci_win));
                if(pm == NULL) {
                    PRINTF ("pci: can't alloc memory for pci memory window\n");
                    return;
                }

                pm->device = pd;
                pm->reg = reg;
                pm->flags = PCI_MAPREG_MEM_TYPE_32BIT;
                pm->size = -(PCI_MAPREG_MEM_ADDR(mask));
                pm->align = pm->size;
                _insertsort_window(&pd->parent->bridge.memspace, pm);
            }
        }

        {
            /* Finally check for Expansion ROM */
            reg = isbridge ? PCI_MAPREG_PPB_ROM : PCI_MAPREG_ROM;
            old = _pci_conf_read(tag, reg);
            _pci_conf_write(tag, reg, 0xfffffffe);
            mask = _pci_conf_read(tag, reg);
            _pci_conf_write(tag, reg, old);

            if (mask != 0 && mask != 0xffffffff) {
                struct pci_win *pm;

                if (_pciverbose >= 3) {
                    _pci_tagprintf (tag, "reg 0x%x = 0x%x\n", reg, mask);
                }

                pm = pmalloc(sizeof(struct pci_win));
                if(pm == NULL) {
                    PRINTF ("pci: can't alloc memory for pci memory window\n");
                    return;
                }
                pm->device = pd;
                pm->reg = reg;
                pm->size = -(PCI_MAPREG_ROM_ADDR(mask));
                            pm->align = pm->size;
                _insertsort_window(&pd->parent->bridge.memspace, pm);
            }
        }
    }
}

static int
_pci_roundup(value, round)
    int value;
    int round;
{
    int result = (value / round) * round;

    if(value % round ||!value)
        result += round;

    return(result);
}

static void
_pci_bus_insert(bus)
    struct pci_bus *bus;
{
    struct pci_bus *pb;

    for(pb = _pci_bushead; pb->next != NULL; pb = pb->next)
        ;;

    pb->next = bus;
}

static void
_pci_device_insert(parent, device)
    struct pci_device *parent;
    struct pci_device *device;
{
    struct pci_device *pd = parent->bridge.child;

    if(pd == NULL) {
        parent->bridge.child = device;
    } else {

        for(; pd->next != NULL; pd = pd->next)
            ;;

        pd->next = device;
    }
}

static void
_pci_query_dev (struct pci_device *dev, int bus, int device, int initialise)
{
	pcitag_t tag;
	pcireg_t id;
	pcireg_t misc;

	tag = _pci_make_tag(bus, device, 0);
	if (!_pci_canscan (tag))
		return;

	if (_pciverbose >= 2)
		_pci_bdfprintf (bus, device, -1, "probe...");
#if defined(LOONGSON_2G5536)||defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A) || defined(LOONGSON_2K)
	id = _pci_conf_read(tag, PCI_ID_REG);

	if (_pciverbose >= 2) {
		PRINTF ("completed\n");
	}
#else
#ifdef CONFIG_LSI_9260
	if ((bus == 2 && device == 0) ||(bus == 3 && device == 0) ||
			(bus == 4 && device == 0))
			delay(2000000);
#endif
	delay(1000);  //fix that the correct id sometimes can not read;
	id = _pci_conf_read(tag, PCI_ID_REG);

	if (_pciverbose >= 2) {
		PRINTF ("completed\n");
	}

	if((bus == 2) && (device ==0))
	{
		if(PCI_VENDOR(id) != 0x1002 && (PCI_PRODUCT(id) != 0x9615))
		  printf("pcie-slot device: vendor:%x product:%x\n",PCI_VENDOR(id),PCI_PRODUCT(id));
	}

#if defined(USE_BMC)
        if (PCI_VENDOR(id) == 0x1a03 && PCI_PRODUCT(id) == 0x2000)
        {
                printf("AST2050's VGA discover !!!!!!!!!!!!!!\n");
        }
#endif
#endif
	if (id == 0 || id == 0xffffffff) {
		return;
	}
	misc = _pci_conf_read(tag, PCI_BHLC_REG);

	if (PCI_HDRTYPE_MULTIFN(misc)) {
		int function;
		for (function = 0; function < 8; function++) {
			tag = _pci_make_tag(bus, device, function);
			id = _pci_conf_read(tag, PCI_ID_REG);
			if (id == 0 || id == 0xffffffff) {
				//return;
				continue;
			}
			_pci_query_dev_func (dev, tag, initialise);
		}
	}
	else {
		_pci_query_dev_func (dev, tag, initialise);
	}
}

pcireg_t
_pci_allocate_mem(dev, size, align)
	struct pci_device *dev;
	vm_size_t size;
        unsigned int align;
{
	pcireg_t address,address1;
#ifdef PCI_ALLOC_MEM_DOWNWARDS
	/* allocate downwards, then round to size boundary */
	address = (dev->bridge.secbus->nextpcimemaddr - size) & ~(align - 1);
	if (address > dev->bridge.secbus->nextpcimemaddr ||
	    address < dev->bridge.secbus->minpcimemaddr) {
		return(-1);
	}
	dev->bridge.secbus->nextpcimemaddr = address;
#else
	/* allocate upwards, then round to size boundary */
	address = (dev->bridge.secbus->minpcimemaddr + align-1) & ~(align - 1);
	address1 = address + size;
	if (address1 > dev->bridge.secbus->nextpcimemaddr ||
	    address1 < dev->bridge.secbus->minpcimemaddr) {
			return(-1);
	}
	dev->bridge.secbus->minpcimemaddr = address1;
#endif	
	return(address);
}



pcireg_t
_pci_allocate_io(struct pci_device *dev, vm_size_t size, unsigned int align)
{
	pcireg_t address,address1;
#ifdef PCI_ALLOC_IO_DOWNWARDS
	/* allocate downwards, then round to size boundary */
	address = (dev->bridge.secbus->nextpciioaddr - align) & ~(align - 1);
	if (address > dev->bridge.secbus->nextpciioaddr ||
            address < dev->bridge.secbus->minpciioaddr) {
		return -1;
	}
	dev->bridge.secbus->nextpciioaddr = address;
#else
	/* allocate upwards, then round to size boundary */
	address=(dev->bridge.secbus->minpciioaddr+align-1)& ~(align - 1);
	address1 = address+size;
	if (address1 > dev->bridge.secbus->nextpciioaddr ||
            address1 < dev->bridge.secbus->minpciioaddr) {
		return -1;
	}
	dev->bridge.secbus->minpciioaddr = address1;
#endif
	return(address);
}

static void
_insertsort_window(pm_list, pm)
    struct pci_win **pm_list;
    struct pci_win *pm;
{
	struct pci_win *pm1, *pm2;

	pm1 = (struct pci_win *)pm_list;
	while((pm2 = pm1->next)) {
		if(pm->align >= pm2->align) {
			break;
		}
		pm1 = pm2;
	}

	pm1->next = pm;
	pm->next = pm2;
}

#ifndef PCI_BIGMEM_ADDRESS
#define PCI_BIGMEM_ADDRESS 0x40000000
#endif
static pci_bigmem_address=PCI_BIGMEM_ADDRESS;
static pci_bigio_address=0x10000;
static void
_pci_setup_windows (struct pci_device *dev)
{
    struct pci_win *pm;
    struct pci_win *next;
    struct pci_device *pd;
    unsigned int align;

    for(pm = dev->bridge.memspace; pm != NULL; pm = next) {

        pd = pm->device;
        next = pm->next;

        if(pd->bridge.child) align = ~pd->bridge.mem_mask+1;
        else align = 1<<(fls(pm->size)-1);

        pm->address = _pci_allocate_mem (dev, pm->size, align);
        if (pm->address == -1) {
	        pci_bigmem_address = (pci_bigmem_address + pm->size-1) & ~(pm->size - 1);
		    pm->address = pci_bigmem_address;
	        pci_bigmem_address += pm->size;

#if 1
            _pci_tagprintf (pd->pa.pa_tag, 
                            "not enough PCI mem space (%d requested)\n",
                            pm->size);
#endif
            //continue;
        }
        if (_pciverbose >= 2)
            _pci_tagprintf (pd->pa.pa_tag, "mem @%p, reg 0x%x %d bytes\n", pm->address, pm->reg, pm->size);

    	if (PCI_ISCLASS(pd->pa.pa_class, PCI_CLASS_BRIDGE, PCI_SUBCLASS_BRIDGE_PCI) && (pm->reg == PCI_MEMBASE_1)) {

            pcireg_t memory;
            
            pm->address = (pm->address + (~pd->bridge.mem_mask))& pd->bridge.mem_mask; //yang23 2013-11-26
            dev->bridge.secbus->minpcimemaddr = pm->address + pm->size; //yang23 2013-11-26

            pd->bridge.secbus->minpcimemaddr = pm->address;
            pd->bridge.secbus->nextpcimemaddr = pm->address + pm->size;

            memory = (((pm->address+pm->size-1) >> 16) << 16) | (pm->address >> 16);
            _pci_conf_write(pd->pa.pa_tag, pm->reg, memory);
			/*set end memory bellow start memory to disable prefectable memory*/
            _pci_conf_write(pd->pa.pa_tag,PCI_PMBASEL_1,0x00000010);

        } else if (pm->reg != PCI_MAPREG_ROM) {
            /* normal memory - expansion rom done below */
            pcireg_t base = _pci_conf_read(pd->pa.pa_tag, pm->reg);
            base = pm->address | (base & ~PCI_MAPREG_MEM_ADDR_MASK);
            _pci_conf_write(pd->pa.pa_tag, pm->reg, base);
        }
    }

    /* Program expansion rom address base after normal memory base,
       to keep DEC ethernet chip happy */
    for (pm = dev->bridge.memspace; pm != NULL; pm = next) {

	pd = pm->device;
#ifdef LOONGSON_2G5536
	if (PCI_ISCLASS(((pd->pa.pa_class)&0xff00ffff), PCI_CLASS_DISPLAY, PCI_SUBCLASS_DISPLAY_VGA))
	{
		vga_dev = pd;
		pd->disable=0;
	}
#else
	if (PCI_ISCLASS(pd->pa.pa_class, PCI_CLASS_DISPLAY, PCI_SUBCLASS_DISPLAY_VGA))
	{
#if defined(USE_BMC)  /* USE_BMC_VGA */
       if (PCI_VENDOR(pd->pa.pa_id) == 0x1a03)
		{
            vga_dev = pd;
            pd->disable=0;
        }
#else
        if ((PCI_VENDOR(pd->pa.pa_id) == 0x1002) && (PCI_PRODUCT(pd->pa.pa_id) == 0x9615))
        {
			printf("vga_dev =:%x\n",vga_dev);
            vga_dev = pd;
            pd->disable=0;
        }else{
		if (PCI_VENDOR(pd->pa.pa_id) == 0x1a03);
		else if (PCI_VENDOR(pd->pa.pa_id) == 0x0014);//ls7a vga
		else {
			printf("pcie_dev :%x vga_dev ==:%x\n",pcie_dev,vga_dev);
	        	pcie_dev = pd;
            		pd->disable=0;
			vga_dev = NULL;
		}
        }
#endif
		printf("pcie_dev :%x vga_dev ==:%x\n",pcie_dev,vga_dev);
	}
#endif
#if 0
	/*
	 * If this is the first VGA card we find, set the BIOS rom
	 * at address c0000 if PCI base address is 0x00000000.
	 */
	if (pm->reg == PCI_MAPREG_ROM && !have_vga &&
	    dev->bridge.secbus->minpcimemaddr == 0 &&
	    (PCI_ISCLASS(pd->pa.pa_class,
		PCI_CLASS_PREHISTORIC, PCI_SUBCLASS_PREHISTORIC_VGA) ||
	    PCI_ISCLASS(pd->pa.pa_class,
		PCI_CLASS_DISPLAY, PCI_SUBCLASS_DISPLAY_VGA))) {
		have_vga = pd->pa.pa_tag;
		pm->address = 0x000c0000;	/* XXX PCI MEM @ 0x000!!! */
	}
#endif
    	if ((PCI_ISCLASS(pd->pa.pa_class, PCI_CLASS_BRIDGE, PCI_SUBCLASS_BRIDGE_PCI) && (pm->reg == PCI_MAPREG_PPB_ROM)) || (pm->reg == PCI_MAPREG_ROM)) {
	    /* expansion rom */
		    if (_pciverbose >= 2){
		        _pci_tagprintf (pd->pa.pa_tag, "exp @%p, %d bytes\n", pm->address, pm->size);
            }
	        _pci_conf_write(pd->pa.pa_tag, pm->reg, pm->address | PCI_MAPREG_TYPE_ROM);
        }

        next = pm->next;
        dev->bridge.memspace = next;
        pfree(pm);
    }

    
    for(pm = dev->bridge.iospace; pm != NULL; pm = next) {

        pd = pm->device;
        next = pm->next;

        if(pd->bridge.child) align = ~pd->bridge.io_mask+1;
        else align = 1<<(fls(pm->size)-1);

        pm->address = _pci_allocate_io (dev, pm->size, align);
        if (pm->address == -1) {
            _pci_tagprintf (pd->pa.pa_tag, 
                            "not enough PCI io space (%d requested)\n", 
                            pm->size);
	        pm->address = pci_bigio_address;
	        pci_bigio_address += pm->size;
        }
        if (_pciverbose >= 2)
		    _pci_tagprintf (pd->pa.pa_tag, "i/o @%p, reg 0x%x %d bytes\n", pm->address, pm->reg, pm->size);

	    if (PCI_ISCLASS(pd->pa.pa_class, PCI_CLASS_BRIDGE, PCI_SUBCLASS_BRIDGE_PCI) &&
           (pm->reg == PCI_IOBASEL_1)) {
	        pcireg_t tmp;

            pd->bridge.secbus->minpciioaddr = pm->address;
            pd->bridge.secbus->nextpciioaddr = pm->address + pm->size;

	        tmp = _pci_conf_read(pd->pa.pa_tag,PCI_IOBASEL_1);
	        tmp &= 0xffff0000;
	        tmp |= (pm->address >> 8) & 0xf0;
	        tmp |= ((pm->address + pm->size-1) & 0xf000);
	        _pci_conf_write(pd->pa.pa_tag,PCI_IOBASEL_1, tmp);

	        tmp = (pm->address >> 16) & 0xffff;
	        tmp |= ((pm->address + pm->size-1) & 0xffff0000);
	        _pci_conf_write(pd->pa.pa_tag,PCI_IOBASEH_1, tmp);

        }
        else {
            _pci_conf_write(pd->pa.pa_tag, pm->reg, pm->address | PCI_MAPREG_TYPE_IO);
        }
        dev->bridge.iospace = next;
        pfree(pm);
    }

    /* Recursive allocate memory for secondary buses */
    for(pd = dev->bridge.child; pd != NULL; pd = pd->next) {
	    if (PCI_ISCLASS(pd->pa.pa_class, PCI_CLASS_BRIDGE, PCI_SUBCLASS_BRIDGE_PCI)) {
            _pci_setup_windows(pd);
        }
    }
}

#ifdef PCIE_GRAPHIC_CARD
bool is_pcie_vga_card()
{
	int ret = 0;
	/*
	tag = _pci_make_tag(bus, dev, fun);
	value = _pci_conf_read(tag, reg);
	*/
	pcitag_t tag;
	unsigned int value;
	tag = _pci_make_tag(1, 0, 0);
	value = _pci_conf_read(tag, 0x08);

	// printf("((((((((((((((( Class_Code=0X%02X))))))))))))))))\n", PCI_CLASS_CODE(value));
	if( (PCI_CLASS_CODE(value) == 0x03) ) // Display Controllers
		ret = 1;
	printf("Is independent_vga_card=%s\n", ( (ret)? "Yes" : "No") );

	return ret;
}
#endif

/*
 * Calculate interrupt routing for a device
 */
static int
_pci_getIntRouting(struct pci_device *dev)
{
	return(dev->intr_routing[PCI_INTLINE_A]);
}

static int
_pci_setupIntRouting(struct pci_device *dev)
{
	int error = -1;
	struct pci_intline_routing *pi;
    
	for(pi = _pci_inthead; pi != NULL; pi = pi->next) {
		if(dev->parent->pa.pa_tag == _pci_make_tag(pi->bus, pi->device, pi->function)) {
			dev->intr_routing[PCI_INTLINE_A] = pi->intline[dev->pa.pa_device][PCI_INTLINE_A];
			dev->intr_routing[PCI_INTLINE_B] = pi->intline[dev->pa.pa_device][PCI_INTLINE_B];
			dev->intr_routing[PCI_INTLINE_C] = pi->intline[dev->pa.pa_device][PCI_INTLINE_C];
			dev->intr_routing[PCI_INTLINE_D] = pi->intline[dev->pa.pa_device][PCI_INTLINE_D];
			error = 0;
			break;
		}
	}
	
	/*
	 * If there where no predefines routing calculate it.
	 */
	if(error == -1) {
		switch(dev->pa.pa_device % 4) {
			case 0:
				dev->intr_routing[PCI_INTLINE_A] = dev->parent->intr_routing[PCI_INTLINE_A];
				dev->intr_routing[PCI_INTLINE_B] = dev->parent->intr_routing[PCI_INTLINE_B];
				dev->intr_routing[PCI_INTLINE_C] = dev->parent->intr_routing[PCI_INTLINE_C];
				dev->intr_routing[PCI_INTLINE_D] = dev->parent->intr_routing[PCI_INTLINE_D];
				error = 0;
				break;
			case 1:
				dev->intr_routing[PCI_INTLINE_A] = dev->parent->intr_routing[PCI_INTLINE_B];
				dev->intr_routing[PCI_INTLINE_B] = dev->parent->intr_routing[PCI_INTLINE_C];
				dev->intr_routing[PCI_INTLINE_C] = dev->parent->intr_routing[PCI_INTLINE_D];
				dev->intr_routing[PCI_INTLINE_D] = dev->parent->intr_routing[PCI_INTLINE_A];
				error = 0;
				break;
			case 2:
				dev->intr_routing[PCI_INTLINE_A] = dev->parent->intr_routing[PCI_INTLINE_C];
				dev->intr_routing[PCI_INTLINE_B] = dev->parent->intr_routing[PCI_INTLINE_D];
				dev->intr_routing[PCI_INTLINE_C] = dev->parent->intr_routing[PCI_INTLINE_A];
				dev->intr_routing[PCI_INTLINE_D] = dev->parent->intr_routing[PCI_INTLINE_B];
				error = 0;
				break;
			case 3:
				dev->intr_routing[PCI_INTLINE_A] = dev->parent->intr_routing[PCI_INTLINE_D];
				dev->intr_routing[PCI_INTLINE_B] = dev->parent->intr_routing[PCI_INTLINE_A];
				dev->intr_routing[PCI_INTLINE_C] = dev->parent->intr_routing[PCI_INTLINE_B];
				dev->intr_routing[PCI_INTLINE_D] = dev->parent->intr_routing[PCI_INTLINE_C];
				error = 0;
				break;
			default:
				break;
		}
	}

	return(error);
}

static void
_pci_setup_devices (struct pci_device *parent, int initialise)
{
    struct pci_device *pd;

    for (pd = parent->bridge.child; pd ; pd = pd->next) {
	/* set device parameters */
	struct pci_bus *pb = pd->pcibus;
	pcitag_t tag = pd->pa.pa_tag;
	pcireg_t cmd, misc, class;
	unsigned int ltim;

	cmd = _pci_conf_read(tag, PCI_COMMAND_STATUS_REG);

	if (initialise) {
	    class = _pci_conf_read(tag, PCI_CLASS_REG);
	    cmd |= PCI_COMMAND_MASTER_ENABLE 
		| PCI_COMMAND_SERR_ENABLE 
		| PCI_COMMAND_PARITY_ENABLE;
	    /* always enable i/o & memory space, in case this card is
	       just snarfing space from the fixed ISA block and doesn't
	       declare separate PCI space. Exception from this is if
	       it is a bridge chip which we will initialize later */
            cmd |= PCI_COMMAND_IO_ENABLE | PCI_COMMAND_MEM_ENABLE;

	    if (pb->fast_b2b)
		cmd |= PCI_COMMAND_BACKTOBACK_ENABLE;

		if(pd->disable)cmd=0;
            _pci_conf_write(tag, PCI_COMMAND_STATUS_REG, cmd);

	    ltim = pd->min_gnt * 33 / 4;
	    ltim = MIN (MAX (pb->def_ltim, ltim), pb->max_ltim);

	    misc = _pci_conf_read (tag, PCI_BHLC_REG);
	    misc = (misc & ~(PCI_LATTIMER_MASK << PCI_LATTIMER_SHIFT))
		   | ((ltim & 0xff) << PCI_LATTIMER_SHIFT);
	    misc = (misc & ~(PCI_CACHELINE_MASK << PCI_CACHELINE_SHIFT))
		   | ((PCI_CACHE_LINE_SIZE & 0xff) << PCI_CACHELINE_SHIFT);
	    _pci_conf_write (tag, PCI_BHLC_REG, misc);
	    
	    if((PCI_CLASS(class) == PCI_CLASS_BRIDGE && PCI_SUBCLASS(class) == PCI_SUBCLASS_BRIDGE_PCI) || pd->bridge.child != NULL) {
		    _pci_setup_devices (pd, initialise); 
	    }
        }
    }
}

void
_pci_businit (int init)
{
	char *v;

	tgt_putchar('P');
	v = getenv("pciverbose");
    tgt_putchar('1');
	if (v) {
	    _pciverbose = atol(v);
	}
    tgt_putchar('2');

	/* intialise the PCI bridge */
	if (/*init*/ 1) {
		SBD_DISPLAY ("PCIH", CHKPNT_PCIH);
		init = _pci_hwinit (init, &def_bus_iot, &def_bus_memt);
		pci_roots = init;
		SBD_DISPLAY ("PCIH", CHKPNT_PCIH);
		if (init < 1)
			return;
	}
	SBD_DISPLAY ("PCID", 0);
	if(monarch_mode) {
		int i;
		struct pci_device *pb;

                if (_pciverbose) {
			printf("setting up %d bus\n", init);
		}
#ifdef LOONGSON_3A2H
		for(i = 0, pb = _pci_head; i < pci_roots; i+=2, pb = pb->next) {
#else
		for(i = 0, pb = _pci_head; i < pci_roots; i++, pb = pb->next) {
#endif
			_pci_scan_dev(pb, i, 0, init);
		}
        	_setup_pcibuses(init);
	}
}

#ifdef BONITOEL_CPCI
static void
_pci_scan_dev(struct pci_device *dev, int bus, int device, int initialise)
{
    int minidev,maxdev;
    minidev=bus?0:13;
    maxdev=bus?32:19;

    for(device=minidev; device < maxdev; device++) //to 19+11, hu mingchang
        {
#if 1
if(getenv("pcistep"))
{
            if(!bus){char str[100];
                printf("pciscan device %d[Y/n]?",device);
                gets(str);
                if(str[0]=='n'){
				int i;
				printf("skip\n");
				for(i=0;i<8;i++)
				_pci_conf_write(_pci_make_tag(0,device,i),4,0);
				continue;
				}
            }
}
#endif
        _pci_query_dev (dev, bus, device, initialise);
    }
}
#elif defined(NC2E)
static void
_pci_scan_dev(struct pci_device *dev, int bus, int device, int initialise)
{
	for(; device < 32; device++) {
		if(!bus && (device==6) && getenv("vga1"))
			{
				int i;
				printf("skip dev 6\n");
				for(i=0;i<8;i++)
				_pci_conf_write(_pci_make_tag(bus,device,i),4,0);
				continue;
			}
		_pci_query_dev (dev, bus, device, initialise);
	}
}
#else
static void
_pci_scan_dev(struct pci_device *dev, int bus, int device, int initialise)
{
	for(; device < 32; device++) {
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
		if(device == 9){//the 1a can not enumerate,it is not a common and standard pci device
			continue;
		}
#endif
		_pci_query_dev (dev, bus, device, initialise);
	}
}
#endif

static void
_setup_pcibuses(int initialise)
{
	struct pci_bus *pb;
	struct pci_device *pd;
	unsigned int def_ltim, max_ltim;
	int i;

	SBD_DISPLAY ("PCIS", CHKPNT_PCIS);

	for(pb = _pci_bushead; pb != NULL; pb = pb->next) {
    
		if (pb->ndev == 0)
			return;

		if (initialise) {
			/* convert largest minimum grant time to cycle count */
/*XXX 66/33 Mhz */	max_ltim = pb->min_gnt * 33 / 4;

			/* now see how much bandwidth is left to distribute */
			if (pb->bandwidth <= 0) {
				if (_pciverbose) {
					_pci_bdfprintf (pb->bus, -1, -1,
					    "WARN: total bandwidth exceeded\n");
				}
				def_ltim = 1;
			}
			else {
				/* calculate a fair share for each device */
				def_ltim = pb->bandwidth / pb->ndev;
				if (def_ltim > pb->max_lat) {
				/* would exceed critical time for some device */
					def_ltim = pb->max_lat;
				}
				/* convert to cycle count */
				def_ltim = def_ltim * 33 / 4;
			}
			/* most devices don't implement bottom three bits */
			def_ltim = (def_ltim + 7) & ~7;
			max_ltim = (max_ltim + 7) & ~7;

			pb->def_ltim = MIN (def_ltim, 255);
			pb->max_ltim = MIN (MAX (max_ltim, def_ltim), 255);
		}
	}

	SBD_DISPLAY ("PCIR", CHKPNT_PCIR);
	_pci_hwreinit ();

	/* setup the individual device windows */
	SBD_DISPLAY ("PCIW", CHKPNT_PCIW);
#ifdef LOONGSON_3A2H
	for(i = 0, pd = _pci_head; i < pci_roots; i+=2, pd = pd->next) {
#else
	for(i = 0, pd = _pci_head; i < pci_roots; i++, pd = pd->next) {
#endif
		_pci_setup_windows (pd);
	}

}


/*
 *  Scan list of configured devices, probe and attach.
 */
void
_pci_devinit (int initialise)
{
	SBD_DISPLAY ("PCID", CHKPNT_PCID);
	if(monarch_mode) {
		int i;
		struct pci_device *pd;

#ifdef LOONGSON_3A2H
		for(i = 0, pd = _pci_head; i < pci_roots; i+=2, pd = pd->next) {
#else
		for(i = 0, pd = _pci_head; i < pci_roots; i++, pd = pd->next) {
#endif
			_pci_setup_devices (pd, initialise);
		}
	}
}

