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
#include <stdbool.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/nppbreg.h>

#include <machine/bus.h>

#include "include/bonito.h"

#include <pmon.h>

#ifdef PCIE_GRAPHIC_CARD
extern bool is_pcie_vga_card();
#endif

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


/* PCI mem regions in PCI space */

/* soft versions of above */
static pcireg_t pci_local_mem_pci_base;


/****************************/
/*initial PCI               */
/****************************/

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
	int i,k;

	if(getenv("newcfg"))newcfg=1;

	if (!initialise) {
		return(0);
	}

	/*
	 *  Allocate and initialize PCI bus heads.
	 */

	/*
	 * PCI Bus 0
	 */
#ifdef PCIE_GRAPHIC_CARD
	if ( is_x4_mode() || is_pcie_vga_card() )
#else
	if (is_x4_mode())
#endif
		k = 1;
	else
		k = 4;
	for (i = 0;i < k;i++) {
		pd = pmalloc(sizeof(struct pci_device));
		pb = pmalloc(sizeof(struct pci_bus));
		if(pd == NULL || pb == NULL) {
			printf("pci: can't alloc memory. pci not initialized\n");
			return(-1);
		}

		pd->pa.pa_flags = PCI_FLAGS_IO_ENABLED | PCI_FLAGS_MEM_ENABLED;
		pd->pa.pa_iot = pmalloc(sizeof(bus_space_tag_t));
		pd->pa.pa_iot->bus_reverse = 1;
		pd->pa.pa_iot->bus_base = BONITO_PCIIO_BASE_VA;
		//printf("pd->pa.pa_iot=%p,bus_base=0x%x\n",pd->pa.pa_iot,pd->pa.pa_iot->bus_base);
		pd->pa.pa_memt = pmalloc(sizeof(bus_space_tag_t));
		pd->pa.pa_memt->bus_reverse = 1;
		//pd->pa.pa_memt->bus_base = PCI_LOCAL_MEM_PCI_BASE;
		pd->pa.pa_memt->bus_base = 0xc0000000;
		pd->pa.pa_dmat = &bus_dmamap_tag;
		pd->bridge.secbus = pb;

		if (i == 0) {
			_pci_head = pd;
		} else if (i == 1){
			_pci_head->next = pd;
		} else if (i == 2){
			_pci_head->next->next = pd;
		} else if (i == 3){
			_pci_head->next->next->next = pd;
		}
	#ifdef LS3_HT /* whd */

		if (i == 0) {
#ifdef PCIE_GRAPHIC_CARD
			if( is_pcie_vga_card() ) {
				pb->minpcimemaddr  = BONITO_PCILO0_BASE; // 0x4000_0000
				pb->nextpcimemaddr = BONITO_PCILO0_BASE+BONITO_PCILO_SIZE; // 0x4000_0000 + 0x4000_0000
				pb->minpciioaddr   = PCI_IO_SPACE_BASE+0x0004000;
				pb->nextpciioaddr  = PCI_IO_SPACE_BASE+ BONITO_PCIIO_SIZE; // 0 + 0x0200_0000
				pb->pci_mem_base   = BONITO_PCILO_BASE_VA;
				pb->pci_io_base    = BONITO_PCIIO_BASE_VA;
			} else {
#endif
				pb->minpcimemaddr  = 0x10000000;
				pb->nextpcimemaddr = 0x12000000;
				pb->minpciioaddr   = 0x18100000;
				pb->nextpciioaddr  = 0x18110000;
				pb->pci_mem_base   = 0x10000000;
				pb->pci_io_base    = 0x18100000;
#ifdef PCIE_GRAPHIC_CARD
			}
#endif
		} else if (i == 1){
			pb->minpcimemaddr  = 0x12000000;
			pb->nextpcimemaddr = 0x14000000;
		        pb->minpciioaddr   = 0x18500000;
		        pb->nextpciioaddr  = 0x18510000;
		        pb->pci_mem_base   = 0x12000000;
		        pb->pci_io_base    = 0x18500000;
		} else if (i == 2){
		        pb->minpcimemaddr  = 0x14000000;
			pb->nextpcimemaddr = 0x16000000;
			pb->minpciioaddr   = 0x18900000;
			pb->nextpciioaddr  = 0x18910000;
			pb->pci_mem_base   = 0x14000000;
			pb->pci_io_base    = 0x18900000;
		} else if (i == 3){
			pb->minpcimemaddr  = 0x16000000;
			pb->nextpcimemaddr = 0x18000000;
			pb->minpciioaddr   = 0x18d00000;
			pb->nextpciioaddr  = 0x18e10000;
			pb->pci_mem_base   = 0x16000000;
			pb->pci_io_base    = 0x18d00000;
		}
#else
		if(newcfg)
		{
		pb->minpcimemaddr  = BONITO_PCILO1_BASE;//??????,???ܺ?linux??????ʼ??ַһ??,????xwindow????ʾ??????
		pb->nextpcimemaddr = BONITO_PCIHI_BASE;
		pd->pa.pa_memt->bus_base = 0xa0000000;
		BONITO_PCIMAP =
		    BONITO_PCIMAP_WIN(0, PCI_MEM_SPACE_PCI_BASE+0x00000000) |
		    BONITO_PCIMAP_WIN(1, PCI_MEM_SPACE_PCI_BASE+0x14000000) |
		    BONITO_PCIMAP_WIN(2, PCI_MEM_SPACE_PCI_BASE+0x18000000) |
		    BONITO_PCIMAP_PCIMAP_2;
		}
		else
		{
		/*pci??ַ??cpu??ַ????,????pci memʱҪioreamapת??,ֱ??or 0xb0000000*/
		pb->minpcimemaddr  = 0x04000000;
		pb->nextpcimemaddr = 0x08000000;
		pd->pa.pa_memt->bus_base = 0xb0000000;
		BONITO_PCIMAP =
		    BONITO_PCIMAP_WIN(0, PCI_MEM_SPACE_PCI_BASE+0x00000000) |
		    BONITO_PCIMAP_WIN(1, PCI_MEM_SPACE_PCI_BASE+0x04000000) |
		    BONITO_PCIMAP_WIN(2, PCI_MEM_SPACE_PCI_BASE+0x08000000) |
		    BONITO_PCIMAP_PCIMAP_2;
		}
		pb->minpciioaddr  = 0x0004000;
		pb->nextpciioaddr = BONITO_PCIIO_SIZE;
		pb->pci_mem_base   = BONITO_PCILO_BASE_VA;
		pb->pci_io_base    = BONITO_PCIIO_BASE_VA;
#endif

		pb->max_lat = 255;
		pb->fast_b2b = 1;
		pb->prefetch = 1;
		pb->bandwidth = 4000000;
		pb->ndev = 1;
		if (i == 0)
			_pci_bushead = pb;
		else if (i == 1)
			_pci_bushead->next = pb;
		else if (i == 2)
			_pci_bushead->next->next = pb;
		else if (i == 3)
			_pci_bushead->next->next->next = pb;

		_pci_bus[_max_pci_bus++] = pd;

		bus_dmamap_tag._dmamap_offs = 0;

/*set pci base0 address and window size*/
		pci_local_mem_pci_base = 0x0;
#ifdef LS3_HT
#else
		BONITO_PCIBASE0 = 0x80000000;
#endif
	}
#ifdef PCIE_GRAPHIC_CARD
	if ( is_x4_mode() || is_pcie_vga_card() )
#else
	if (is_x4_mode())
#endif
		return(1);
	else
		return(7);
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
	CPU_IOFlushDCache(adr, size, rw);
}

