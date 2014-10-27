/**\file
 *  This file defines the wrapper for the platform/OS related functions
 *  The function definitions needs to be modified according to the platform 
 *  and the Operating system used.
 *  This file should be handled with greatest care while porting the driver
 *  to a different platform running different operating system other than
 *  Linux 2.6.xx.
 * \internal
 * ----------------------------REVISION HISTORY-----------------------------
 * Synopsys			01/Aug/2007			Created
 */
 
#include "synopGMAC_plat.h"

/**
  * This is a wrapper function for Memory allocation routine. In linux Kernel 
  * it it kmalloc function
  * @param[in] bytes in bytes to allocate
  */


#if defined(LOONGSON_2G1A)
dma_addr_t __attribute__((weak)) gmac_dmamap(unsigned long va,size_t size)
{
	unsigned long dma_adr;
	if(va >= 0x84000000 && va <= 0x8fffffff)
		dma_adr = va - 0x74000000;
	else
		printf("error!!! the address is out of the rang that 1F can map ,you can change the pci_map\n");
	return dma_adr;
}
#endif

void *plat_alloc_memory(u32 bytes) 
{
	return (void*)malloc((size_t)bytes, M_DEVBUF, M_DONTWAIT);
}

/**
  * This is a wrapper function for consistent dma-able Memory allocation routine. 
  * In linux Kernel, it depends on pci dev structure
  * @param[in] bytes in bytes to allocate
  */

#if defined(LOONGSON_2G1A)
void *plat_alloc_consistent_dmaable_memory(u32 size, u32 *addr)
{
	void *buf;
	buf = (void*)malloc((size_t)size, M_DEVBUF, M_DONTWAIT);
	CPU_IOFlushDCache( buf,size, SYNC_W);

	*addr =gmac_dmamap(buf,size);
	buf = (unsigned char *)CACHED_TO_UNCACHED(buf);
	return buf;
}
#endif

//sw to-be clean
/*
void *plat_alloc_consistent_dmaable_memory(struct pci_dev *pcidev, u32 size, u32 *addr) 
{
 return (pci_alloc_consistent (pcidev,size,addr));
}
*/

/**
  * This is a wrapper function for freeing consistent dma-able Memory.
  * In linux Kernel, it depends on pci dev structure
  * @param[in] bytes in bytes to allocate
  */

//sw to-be clean
/*
void plat_free_consistent_dmaable_memory(struct pci_dev *pcidev, u32 size, void * addr,u32 dma_addr) 
{
 pci_free_consistent (pcidev,size,addr,dma_addr);
 return;
}
*/


/**
  * This is a wrapper function for Memory free routine. In linux Kernel 
  * it it kfree function
  * @param[in] buffer pointer to be freed
  */
void plat_free_memory(void *buffer) 
{
	free(buffer,M_DEVBUF);
	return ;
}

#if defined(LOONGSON_2G1A)
dma_addr_t plat_dma_map_single(void *hwdev, void *ptr,
				size_t size, int direction)
{
	unsigned long addr = (unsigned long) ptr;
	CPU_IOFlushDCache(addr,size, direction);
	return gmac_dmamap(addr,size);
}
#endif

/**
  * This is a wrapper function for platform dependent delay 
  * Take care while passing the argument to this function 
  * @param[in] buffer pointer to be freed
  */
void plat_delay(u32 delay)
{
	while (delay--);
	return;
}


