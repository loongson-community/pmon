/*	$Id: pci_machdep.c,v 1.1.1.1 2006/09/14 01:59:09 root Exp $ */

/*
 * Copyright (c) 2001 Opsycon AB  (www.opsycon.se)
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
 *	This product includes software developed by Opsycon AB, Sweden.
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

#include <sys/param.h>
#include <sys/device.h>
#include <sys/systm.h>

#include <sys/malloc.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/nppbreg.h>

#include <machine/bus.h>

#include "include/bonito.h"

#include <pmon.h>

extern void *pmalloc __P((size_t ));
#if  (PCI_IDSEL_CS5536 != 0)
#include <include/cs5536_pci.h>
extern pcireg_t cs5536_pci_conf_readn(int function, int reg, int width);
extern int cs5536_pci_conf_writen(int function, int reg, int width, pcireg_t value);
#endif
extern void *pmalloc __P((size_t ));

extern int _pciverbose;

extern char hwethadr[6];
struct pci_device *_pci_bus[16];
int _max_pci_bus = 0;
extern int pci_probe_only;


/* PCI mem regions in PCI space */

/* soft versions of above */
static pcireg_t pci_local_mem_pci_base;


/****************************/
/*initial PCI               */
/****************************/

#define PCI0_CONFIG(dev,reg) *(volatile int *)(0xba000000+(dev<<11)+reg) 
int
_pci_hwinit(initialise, iot, memt)
	int initialise;
	bus_space_tag_t iot;
	bus_space_tag_t memt;
{
	/*pcireg_t stat;*/
	struct pci_device *pd;
	struct pci_bus *pb;
	int newcfg=0;
	char *env;
	SBD_DISPLAY ("HW-0", 0);
	int waitdev = 0, timeout;
	if(getenv("newcfg"))newcfg=1;

	if ((env = getenv("pci_probe_only")))
	  pci_probe_only = strtoul(env, 0, 0);

	if((env = getenv("pcidelay")))
	{
		char *endp;
		int dev;
		int count0;
		unsigned int pcidelay;

		pcidelay= strtoul(env, &endp, 0);
		if(endp && endp[0] && endp[1])
		{
			endp++;
			count0 = read_c0_count();

			for(timeout = 0; timeout < pcidelay;)
			{
				dev = strtoul(endp, &endp, 0);
				PCI0_CONFIG(dev, 0x10) = 0x10000000;
				while(!(*(volatile int *)0xb000000c & 0x40) && timeout < pcidelay)
				{
					if(read_c0_count() - count0 > 400000000)
					{
						timeout++;
						count0 = read_c0_count();
					}
				}
				printf("dev %d link 0x%x\n", dev, *(volatile int *)0xb000000c);
				PCI0_CONFIG(dev, 0x10) = 0x00000000;
				if(endp && endp[0] && endp[1])
					endp++;
				else
					break;
			}
		}
		else
			delay(pcidelay);
	}

	if (!initialise) {
		return(0);
	}
	SBD_DISPLAY ("HW-1", 0);

	/*
	 *  Allocate and initialize PCI bus heads.
	 */

	/*
	 * PCI Bus 0
	 */
	pd = pmalloc(sizeof(struct pci_device));
	pb = pmalloc(sizeof(struct pci_bus));
	if(pd == NULL || pb == NULL) {
		printf("pci: can't alloc memory. pci not initialized\n");
		return(-1);
	}
	SBD_DISPLAY ("HW-2", 0);

	pd->pa.pa_flags = PCI_FLAGS_IO_ENABLED | PCI_FLAGS_MEM_ENABLED;
	pd->pa.pa_iot = pmalloc(sizeof(bus_space_tag_t));
	pd->pa.pa_iot->bus_reverse = 1;
	pd->pa.pa_iot->bus_base = BONITO_PCIIO_BASE_VA;
	pd->pa.pa_memt = pmalloc(sizeof(bus_space_tag_t));
	pd->pa.pa_memt->bus_reverse = 1;
	pd->pa.pa_memt->bus_base = 0xc0000000;
	pd->pa.pa_dmat = &bus_dmamap_tag;
	pd->bridge.secbus = pb;
	_pci_head = pd;
	SBD_DISPLAY ("HW-3", 0);

	if(pci_probe_only)
	{
		pb->minpcimemaddr  = 0x40100000;
		pb->nextpcimemaddr = 0x80000000;
	}
	else
	{
		pb->minpcimemaddr  = 0x10000000;
		pb->nextpcimemaddr = 0x18000000;
	}

	pb->minpciioaddr   = PCI_IO_SPACE_BASE+0x0004000;
	pb->nextpciioaddr  = PCI_IO_SPACE_BASE+ BONITO_PCIIO_SIZE; // 0 + 0x0200_0000
	pb->pci_mem_base   = BONITO_PCILO_BASE_VA;
	pb->pci_io_base    = BONITO_PCIIO_BASE_VA;
#if 0 //low pci mem base
	pd->pa.pa_memt->bus_base = 0xa0000000;
	pb->minpcimemaddr  = 0x10000000;
	pb->nextpcimemaddr = 0x17ffffff;
	pb->minpciioaddr   = 0x18000000;
	pb->nextpciioaddr  = 0x1800ffff;
	pb->pci_mem_base   = 0x10000000;
	pb->pci_io_base    = 0x18100000;
#endif
	pb->max_lat = 255;
	pb->fast_b2b = 1;
	pb->prefetch = 1;
	pb->bandwidth = 4000000;
	pb->ndev = 1;
	_pci_bushead = pb;
	_pci_bus[_max_pci_bus++] = pd;

	SBD_DISPLAY ("HW-5", 0);
	
	bus_dmamap_tag._dmamap_offs = 0;

	/*set pci base0 address and window size*/
	pci_local_mem_pci_base = 0x0;

	return(1);
}

/*
 * Called to reinitialise the bridge after we've scanned each PCI device
 * and know what is possible. We also set up the interrupt controller
 * routing and level control registers.
 */
void
_pci_hwreinit (void)
{
}

void
_pci_flush (void)
{
}

/*
 *  Map the CPU virtual address of an area of local memory to a PCI
 *  address that can be used by a PCI bus master to access it.
 */
vm_offset_t
_pci_dmamap(va, len)
	vm_offset_t va;
	unsigned int len;
{
#if 0
	return(VA_TO_PA(va) + bus_dmamap_tag._dmamap_offs);
#endif
	return(pci_local_mem_pci_base + VA_TO_PA (va));
}


/*
 *  Map the PCI address of an area of local memory to a CPU physical
 *  address.
 */
vm_offset_t
_pci_cpumap(pcia, len)
	vm_offset_t pcia;
	unsigned int len;
{
	return PA_TO_VA(pcia - pci_local_mem_pci_base);
}


/*
 *  Make pci tag from bus, device and function data.
 */
pcitag_t
_pci_make_tag(bus, device, function)
	int bus;
	int device;
	int function;
{
	pcitag_t tag;

	tag = (bus << 16) | (device << 11) | (function << 8);
	return(tag);
}

/*
 *  Break up a pci tag to bus, device function components.
 */
void
_pci_break_tag(tag, busp, devicep, functionp)
	pcitag_t tag;
	int *busp;
	int *devicep;
	int *functionp;
{
	if (busp) {
		*busp = (tag >> 16) & 255;
	}
	if (devicep) {
		*devicep = (tag >> 11) & 31;
	}
	if (functionp) {
		*functionp = (tag >> 8) & 7;
	}
}

int
_pci_canscan (pcitag_t tag)
{
	int bus, device, function;

	_pci_break_tag (tag, &bus, &device, &function); 
	return (1);
}

void
pci_sync_cache(p, adr, size, rw)
	void *p;
	vm_offset_t adr;
	size_t size;
	int rw;
{
//	CPU_IOFlushDCache(adr, size, rw);
}

int pci_get_busno(struct pci_device *pd, int bus)
{
	int ret = bus + 1;
	if(!pd->pa.pa_bus)
	{
		switch(pd->pa.pa_device)
		{
			case 9:
				ret = 1;
				break;
			case 0xa:
				ret = 4;
				break;
			case 0xb:
				ret = 8;
				break;
			case 0xc:
				ret = 0xc;
				break;
			case 0xd:
				ret = 0x10;
				break;
			case 0xe:
				ret = 0x14;
				break;
		}
	}
	
	return ret;
}

extern struct pci_config_data pci_config_array[];
extern int pci_config_array_size;
static char pci_dev_index[0x12*4];

int __attribute__ ((constructor)) build_pci() {
	int i;
	for(i = 0;i < pci_config_array_size ;i++){
		pci_dev_index [((pci_config_array[i].dev<<2) |pci_config_array[i].func) ] = i;
	}
	return 0;
}

pcireg_t pci_alloc_fixmemio(struct pci_win *pm)
{
	int idx;
	struct pci_device *pd = pm->device;
	int reg = pm->reg;
	size_t size;

	if(!pd->pa.pa_bus)
	{
		idx = pci_dev_index [((pd->pa.pa_device<<2) |pd->pa.pa_function)];
		if(idx)
		{
			if(pci_config_array[idx].type == PCI_DEV)
			{
				if(reg == 0x10) 
				{
					size = pci_config_array[idx].mem_end - pci_config_array[idx].mem_start + 1;
					if(size < pm->size || pci_config_array[idx].mem_start & (pm->size - 1)) 
						_pci_tagprintf (pd->pa.pa_tag, "alloced resource size or alignment failed,want 0x%x/0x%x, got 0x%x/0x%x\n", pci_config_array[idx].mem_start, size, pci_config_array[idx].mem_start & ~(pm->size -1), pm->size);
					return pci_config_array[idx].mem_start;
				}
			}
			else
			{
				if(pci_probe_only)
					return -1;

				if(reg == PCI_MEMBASE_1) 
				{
					size = pci_config_array[idx].mem_end - pci_config_array[idx].mem_start + 1;
					if(size < pm->size || pci_config_array[idx].mem_start & (pm->size - 1)) 
						_pci_tagprintf (pd->pa.pa_tag, "alloced resource size or alignment failed,want 0x%x/0x%x, got 0x%x/0x%x\n", pci_config_array[idx].mem_start, size, pci_config_array[idx].mem_start & ~(pm->size -1), pm->size);
					pm->size = size;
					return pci_config_array[idx].mem_start;
				}

				if(reg == PCI_IOBASEL_1)
				{
					size = pci_config_array[idx].io_end - pci_config_array[idx].io_start + 1;
					if(size < pm->size || pci_config_array[idx].mem_start & (pm->size - 1)) 
						_pci_tagprintf (pd->pa.pa_tag, "alloced resource size or alignment failed,want 0x%x/0x%x, got 0x%x/0x%x\n", pci_config_array[idx].io_start & LS2K_PCI_IO_MASK, size, pci_config_array[idx].io_start & ~(pm->size -1) & LS2K_PCI_IO_MASK, pm->size);
					pm->size = size;
					return  pci_config_array[idx].io_start & LS2K_PCI_IO_MASK ;
				}
			}
		}
	}
	return -1;
}
