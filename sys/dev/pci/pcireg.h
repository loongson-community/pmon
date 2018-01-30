/*	$OpenBSD: pcireg.h,v 1.17 2001/05/08 19:47:43 mickey Exp $	*/
/*	$NetBSD: pcireg.h,v 1.26 2000/05/10 16:58:42 thorpej Exp $	*/

/*
 * Copyright (c) 1995, 1996 Christopher G. Demetriou.  All rights reserved.
 * Copyright (c) 1994, 1996 Charles Hannum.  All rights reserved.
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
 *	This product includes software developed by Charles Hannum.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DEV_PCI_PCIREG_H_
#define	_DEV_PCI_PCIREG_H_

/*
 * Standardized PCI configuration information
 *
 * XXX This is not complete.
 */

/*
 * Device identification register; contains a vendor ID and a device ID.
 */
#define	PCI_ID_REG			0x00

typedef u_int16_t pci_vendor_id_t;
typedef u_int16_t pci_product_id_t;

#define	PCI_VENDOR_SHIFT			0
#define	PCI_VENDOR_MASK				0xffff
#define	PCI_VENDOR(id) \
	    (((id) >> PCI_VENDOR_SHIFT) & PCI_VENDOR_MASK)

#define	PCI_PRODUCT_SHIFT			16
#define	PCI_PRODUCT_MASK			0xffff
#define	PCI_PRODUCT(id) \
	    (((id) >> PCI_PRODUCT_SHIFT) & PCI_PRODUCT_MASK)

#ifdef PCIE_GRAPHIC_CARD
#define PCI_CLASS_CODE_SHIFT           24
#define PCI_CLASS_CODE_MASK            0xff
#define PCI_CLASS_CODE(cl) \
        (((cl) >> PCI_CLASS_CODE_SHIFT) & PCI_CLASS_CODE_MASK)
#endif

/*
 * Command and status register.
 */
#define	PCI_COMMAND_STATUS_REG			0x04

#define	PCI_COMMAND_IO_ENABLE			0x00000001
#define	PCI_COMMAND_MEM_ENABLE			0x00000002
#define	PCI_COMMAND_MASTER_ENABLE		0x00000004
#define	PCI_COMMAND_SPECIAL_ENABLE		0x00000008
#define	PCI_COMMAND_INVALIDATE_ENABLE		0x00000010
#define	PCI_COMMAND_PALETTE_ENABLE		0x00000020
#define	PCI_COMMAND_PARITY_ENABLE		0x00000040
#define	PCI_COMMAND_STEPPING_ENABLE		0x00000080
#define	PCI_COMMAND_SERR_ENABLE			0x00000100
#define	PCI_COMMAND_BACKTOBACK_ENABLE		0x00000200

#define	PCI_STATUS_CAPLIST_SUPPORT		0x00100000
#define	PCI_STATUS_66MHZ_SUPPORT		0x00200000
#define	PCI_STATUS_UDF_SUPPORT			0x00400000
#define	PCI_STATUS_BACKTOBACK_SUPPORT		0x00800000
#define	PCI_STATUS_PARITY_ERROR			0x01000000
#define	PCI_STATUS_DEVSEL_FAST			0x00000000
#define	PCI_STATUS_DEVSEL_MEDIUM		0x02000000
#define	PCI_STATUS_DEVSEL_SLOW			0x04000000
#define	PCI_STATUS_DEVSEL_MASK			0x06000000
#define	PCI_STATUS_TARGET_TARGET_ABORT		0x08000000
#define	PCI_STATUS_MASTER_TARGET_ABORT		0x10000000
#define	PCI_STATUS_MASTER_ABORT			0x20000000
#define	PCI_STATUS_SPECIAL_ERROR		0x40000000
#define	PCI_STATUS_PARITY_DETECT		0x80000000

/*
 * PCI Class and Revision Register; defines type and revision of device.
 */
#define	PCI_CLASS_REG			0x08

typedef u_int8_t pci_class_t;
typedef u_int8_t pci_subclass_t;
typedef u_int8_t pci_interface_t;
typedef u_int8_t pci_revision_t;

#define	PCI_CLASS_SHIFT				24
#define	PCI_CLASS_MASK				0xff
#define	PCI_CLASS(cr) \
	    (((cr) >> PCI_CLASS_SHIFT) & PCI_CLASS_MASK)

#define	PCI_SUBCLASS_SHIFT			16
#define	PCI_SUBCLASS_MASK			0xff
#define	PCI_SUBCLASS(cr) \
	    (((cr) >> PCI_SUBCLASS_SHIFT) & PCI_SUBCLASS_MASK)

#define PCI_ISCLASS(what, class, subclass) \
	(((what) & 0xffff0000) == (class << 24 | subclass << 16))

#define	PCI_INTERFACE_SHIFT			8
#define	PCI_INTERFACE_MASK			0xff
#define	PCI_INTERFACE(cr) \
	    (((cr) >> PCI_INTERFACE_SHIFT) & PCI_INTERFACE_MASK)

#define	PCI_REVISION_SHIFT			0
#define	PCI_REVISION_MASK			0xff
#define	PCI_REVISION(cr) \
	    (((cr) >> PCI_REVISION_SHIFT) & PCI_REVISION_MASK)

/* base classes */
#define	PCI_CLASS_PREHISTORIC			0x00
#define	PCI_CLASS_MASS_STORAGE			0x01
#define	PCI_CLASS_NETWORK			0x02
#define	PCI_CLASS_DISPLAY			0x03
#define	PCI_CLASS_MULTIMEDIA			0x04
#define	PCI_CLASS_MEMORY			0x05
#define	PCI_CLASS_BRIDGE			0x06
#define	PCI_CLASS_COMMUNICATIONS		0x07
#define	PCI_CLASS_SYSTEM			0x08
#define	PCI_CLASS_INPUT				0x09
#define	PCI_CLASS_DOCK				0x0a
#define	PCI_CLASS_PROCESSOR			0x0b
#define	PCI_CLASS_SERIALBUS			0x0c
#define	PCI_CLASS_WIRELESS			0x0d
#define	PCI_CLASS_I2O				0x0e
#define	PCI_CLASS_SATCOM			0x0f
#define	PCI_CLASS_CRYPTO			0x10
#define	PCI_CLASS_DASP				0x11
#define	PCI_CLASS_UNDEFINED			0xff

/* 0x00 prehistoric subclasses */
#define	PCI_SUBCLASS_PREHISTORIC_MISC		0x00
#define	PCI_SUBCLASS_PREHISTORIC_VGA		0x01

/* 0x01 mass storage subclasses */
#define	PCI_SUBCLASS_MASS_STORAGE_SCSI		0x00
#define	PCI_SUBCLASS_MASS_STORAGE_IDE		0x01
#define	PCI_SUBCLASS_MASS_STORAGE_FLOPPY	0x02
#define	PCI_SUBCLASS_MASS_STORAGE_IPI		0x03
#define	PCI_SUBCLASS_MASS_STORAGE_RAID		0x04
#define	PCI_SUBCLASS_MASS_STORAGE_ATA		0x05
#define	PCI_SUBCLASS_MASS_STORAGE_MISC		0x80

/* 0x02 network subclasses */
#define	PCI_SUBCLASS_NETWORK_ETHERNET		0x00
#define	PCI_SUBCLASS_NETWORK_TOKENRING		0x01
#define	PCI_SUBCLASS_NETWORK_FDDI		0x02
#define	PCI_SUBCLASS_NETWORK_ATM		0x03
#define	PCI_SUBCLASS_NETWORK_ISDN		0x04
#define	PCI_SUBCLASS_NETWORK_WORLDFIP		0x05
#define	PCI_SUBCLASS_NETWORK_PCIMGMULTICOMP	0x06
#define	PCI_SUBCLASS_NETWORK_MISC		0x80

/* 0x03 display subclasses */
#define	PCI_SUBCLASS_DISPLAY_VGA		0x00
#define	PCI_SUBCLASS_DISPLAY_XGA		0x01
#define	PCI_SUBCLASS_DISPLAY_3D			0x02
#define	PCI_SUBCLASS_DISPLAY_MISC		0x80

/* 0x04 multimedia subclasses */
#define	PCI_SUBCLASS_MULTIMEDIA_VIDEO		0x00
#define	PCI_SUBCLASS_MULTIMEDIA_AUDIO		0x01
#define	PCI_SUBCLASS_MULTIMEDIA_TELEPHONY	0x02
#define	PCI_SUBCLASS_MULTIMEDIA_MISC		0x80

/* 0x05 memory subclasses */
#define	PCI_SUBCLASS_MEMORY_RAM			0x00
#define	PCI_SUBCLASS_MEMORY_FLASH		0x01
#define	PCI_SUBCLASS_MEMORY_MISC		0x80

/* 0x06 bridge subclasses */
#define	PCI_SUBCLASS_BRIDGE_HOST		0x00
#define	PCI_SUBCLASS_BRIDGE_ISA			0x01
#define	PCI_SUBCLASS_BRIDGE_EISA		0x02
#define	PCI_SUBCLASS_BRIDGE_MC			0x03
#define	PCI_SUBCLASS_BRIDGE_PCI			0x04
#define	PCI_SUBCLASS_BRIDGE_PCMCIA		0x05
#define	PCI_SUBCLASS_BRIDGE_NUBUS		0x06
#define	PCI_SUBCLASS_BRIDGE_CARDBUS		0x07
#define	PCI_SUBCLASS_BRIDGE_RACEWAY		0x08
#define	PCI_SUBCLASS_BRIDGE_STPCI		0x09
#define	PCI_SUBCLASS_BRIDGE_INFINIBAND		0x0a
#define	PCI_SUBCLASS_BRIDGE_MISC		0x80

/* 0x07 communications subclasses */
#define	PCI_SUBCLASS_COMMUNICATIONS_SERIAL	0x00
#define	PCI_SUBCLASS_COMMUNICATIONS_PARALLEL	0x01
#define	PCI_SUBCLASS_COMMUNICATIONS_MPSERIAL	0x02
#define	PCI_SUBCLASS_COMMUNICATIONS_MODEM	0x03
#define	PCI_SUBCLASS_COMMUNICATIONS_MISC	0x80

/* 0x08 system subclasses */
#define	PCI_SUBCLASS_SYSTEM_PIC			0x00
#define	PCI_SUBCLASS_SYSTEM_DMA			0x01
#define	PCI_SUBCLASS_SYSTEM_TIMER		0x02
#define	PCI_SUBCLASS_SYSTEM_RTC			0x03
#define	PCI_SUBCLASS_SYSTEM_PCIHOTPLUG		0x04
#define	PCI_SUBCLASS_SYSTEM_MISC		0x80

/* 0x09 input subclasses */
#define	PCI_SUBCLASS_INPUT_KEYBOARD		0x00
#define	PCI_SUBCLASS_INPUT_DIGITIZER		0x01
#define	PCI_SUBCLASS_INPUT_MOUSE		0x02
#define	PCI_SUBCLASS_INPUT_SCANNER		0x03
#define	PCI_SUBCLASS_INPUT_GAMEPORT		0x04
#define	PCI_SUBCLASS_INPUT_MISC			0x80

/* 0x0a dock subclasses */
#define	PCI_SUBCLASS_DOCK_GENERIC		0x00
#define	PCI_SUBCLASS_DOCK_MISC			0x80

/* 0x0b processor subclasses */
#define	PCI_SUBCLASS_PROCESSOR_386		0x00
#define	PCI_SUBCLASS_PROCESSOR_486		0x01
#define	PCI_SUBCLASS_PROCESSOR_PENTIUM		0x02
#define	PCI_SUBCLASS_PROCESSOR_ALPHA		0x10
#define	PCI_SUBCLASS_PROCESSOR_POWERPC		0x20
#define	PCI_SUBCLASS_PROCESSOR_MIPS		0x30
#define	PCI_SUBCLASS_PROCESSOR_COPROC		0x40

/* 0x0c serial bus subclasses */
#define	PCI_SUBCLASS_SERIALBUS_FIREWIRE		0x00
#define	PCI_SUBCLASS_SERIALBUS_ACCESS		0x01
#define	PCI_SUBCLASS_SERIALBUS_SSA		0x02
#define	PCI_SUBCLASS_SERIALBUS_USB		0x03
#define	PCI_SUBCLASS_SERIALBUS_FIBER		0x04
#define	PCI_SUBCLASS_SERIALBUS_SMBUS		0x05
#define	PCI_SUBCLASS_SERIALBUS_INFINIBAND	0x06
#define	PCI_SUBCLASS_SERIALBUS_IPMI		0x07
#define	PCI_SUBCLASS_SERIALBUS_SERCOS		0x08
#define	PCI_SUBCLASS_SERIALBUS_CANBUS		0x09

/* 0x0d wireless subclasses */
#define	PCI_SUBCLASS_WIRELESS_IRDA		0x00
#define	PCI_SUBCLASS_WIRELESS_CONSUMERIR	0x01
#define	PCI_SUBCLASS_WIRELESS_RF		0x10
#define	PCI_SUBCLASS_WIRELESS_MISC		0x80

/* 0x0e I2O (Intelligent I/O) subclasses */
#define	PCI_SUBCLASS_I2O_STANDARD		0x00

/* 0x0f satellite communication subclasses */
/*	PCI_SUBCLASS_SATCOM_???			0x00    / * XXX ??? */
#define	PCI_SUBCLASS_SATCOM_TV			0x01
#define	PCI_SUBCLASS_SATCOM_AUDIO		0x02
#define	PCI_SUBCLASS_SATCOM_VOICE		0x03
#define	PCI_SUBCLASS_SATCOM_DATA		0x04

/* 0x10 encryption/decryption subclasses */
#define	PCI_SUBCLASS_CRYPTO_NETCOMP		0x00
#define	PCI_SUBCLASS_CRYPTO_ENTERTAINMENT	0x10
#define	PCI_SUBCLASS_CRYPTO_MISC		0x80

/* 0x11 data acquisition and signal processing subclasses */
#define	PCI_SUBCLASS_DASP_DPIO			0x00
#define	PCI_SUBCLASS_DASP_TIMEFREQ		0x01
#define	PCI_SUBCLASS_DASP_MISC			0x80

/*
 * PCI BIST/Header Type/Latency Timer/Cache Line Size Register.
 */
#define	PCI_BHLC_REG			0x0c

#define	PCI_BIST_SHIFT				24
#define	PCI_BIST_MASK				0xff
#define	PCI_BIST(bhlcr) \
	    (((bhlcr) >> PCI_BIST_SHIFT) & PCI_BIST_MASK)

#define	PCI_HDRTYPE_SHIFT			16
#define	PCI_HDRTYPE_MASK			0xff
#define	PCI_HDRTYPE(bhlcr) \
	    (((bhlcr) >> PCI_HDRTYPE_SHIFT) & PCI_HDRTYPE_MASK)

#define PCI_HDRTYPE_TYPE(bhlcr) \
	    (PCI_HDRTYPE(bhlcr) & 0x7f)
#define	PCI_HDRTYPE_MULTIFN(bhlcr) \
	    ((PCI_HDRTYPE(bhlcr) & 0x80) != 0)

#define	PCI_LATTIMER_SHIFT			8
#define	PCI_LATTIMER_MASK			0xff
#define	PCI_LATTIMER(bhlcr) \
	    (((bhlcr) >> PCI_LATTIMER_SHIFT) & PCI_LATTIMER_MASK)

#define	PCI_CACHELINE_SHIFT			0
#define	PCI_CACHELINE_MASK			0xff
#define	PCI_CACHELINE(bhlcr) \
	    (((bhlcr) >> PCI_CACHELINE_SHIFT) & PCI_CACHELINE_MASK)

/* config registers for header type 0 devices */

#define PCI_MAPS	0x10
#define PCI_CARDBUSCIS	0x28
#define PCI_SUBVEND_0	0x2c
#define PCI_SUBDEV_0	0x2e
#define PCI_INTLINE	0x3c
#define PCI_INTPIN	0x3d
#define PCI_MINGNT	0x3e
#define PCI_MAXLAT	0x3f

/* config registers for header type 1 devices */

#define PCI_SECSTAT_1	0 /**/

#define PCI_PRIBUS_1	0x18
#define PCI_SECBUS_1	0x19
#define PCI_SUBBUS_1	0x1a
#define PCI_SECLAT_1	0x1b

#define PCI_IOBASEL_1	0x1c
#define PCI_IOLIMITL_1	0x1d
#define PCI_IOBASEH_1	0x30 /**/
#define PCI_IOLIMITH_1	0x32 /**/ 

#define PCI_MEMBASE_1	0x20
#define PCI_MEMLIMIT_1	0x22

#define PCI_PMBASEL_1	0x24
#define PCI_PMLIMITL_1	0x26
#define PCI_PMBASEH_1	0 /**/
#define PCI_PMLIMITH_1	0 /**/

#define PCI_BRIDGECTL_1 0 /**/

#define PCI_SUBVEND_1	0x34
#define PCI_SUBDEV_1	0x36

/* config registers for header type 2 devices */

#define PCI_SECSTAT_2	0x16

#define PCI_PRIBUS_2	0x18
#define PCI_SECBUS_2	0x19
#define PCI_SUBBUS_2	0x1a
#define PCI_SECLAT_2	0x1b

#define PCI_MEMBASE0_2	0x1c
#define PCI_MEMLIMIT0_2 0x20
#define PCI_MEMBASE1_2	0x24
#define PCI_MEMLIMIT1_2 0x28
#define PCI_IOBASE0_2	0x2c
#define PCI_IOLIMIT0_2	0x30
#define PCI_IOBASE1_2	0x34
#define PCI_IOLIMIT1_2	0x38

#define PCI_BRIDGECTL_2 0x3e

#define PCI_SUBVEND_2	0x40
#define PCI_SUBDEV_2	0x42

#define PCI_PCCARDIF_2	0x44

/*
 * Mapping registers
 */
#define	PCI_MAPREG_START		0x10
#define	PCI_MAPREG_END			0x28
#define	PCI_MAPREG_PPB_END		0x18
#define	PCI_MAPREG_ROM			0x30
#define	PCI_MAPREG_PPB_ROM	    0x38
#define	PCI_MAPREG_PCB_END		0x14

#define	PCI_MAPREG_TYPE(mr)						\
	    ((mr) & PCI_MAPREG_TYPE_MASK)
#define	PCI_MAPREG_TYPE_MASK			0x00000001

#define	PCI_MAPREG_TYPE_MEM			0x00000000
#define	PCI_MAPREG_TYPE_IO			0x00000001
#define	PCI_MAPREG_TYPE_ROM			0x00000001

#define	PCI_MAPREG_MEM_TYPE(mr)						\
	    ((mr) & PCI_MAPREG_MEM_TYPE_MASK)
#define	PCI_MAPREG_MEM_TYPE_MASK		0x00000006

#define	PCI_MAPREG_MEM_TYPE_32BIT		0x00000000
#define	PCI_MAPREG_MEM_TYPE_32BIT_1M		0x00000002
#define	PCI_MAPREG_MEM_TYPE_64BIT		0x00000004

#define	PCI_MAPREG_MEM_CACHEABLE(mr)					\
	    (((mr) & PCI_MAPREG_MEM_CACHEABLE_MASK) != 0)
#define	PCI_MAPREG_MEM_CACHEABLE_MASK		0x00000008

#define	PCI_MAPREG_MEM_PREFETCHABLE(mr)					\
	    (((mr) & PCI_MAPREG_MEM_PREFETCHABLE_MASK) != 0)
#define	PCI_MAPREG_MEM_PREFETCHABLE_MASK	0x00000008

#define	PCI_MAPREG_MEM_ADDR(mr)						\
	    ((mr) & PCI_MAPREG_MEM_ADDR_MASK)
#define	PCI_MAPREG_MEM_SIZE(mr)						\
	    (PCI_MAPREG_MEM_ADDR(mr) & -PCI_MAPREG_MEM_ADDR(mr))
#define	PCI_MAPREG_MEM_ADDR_MASK		0xfffffff0

#define	PCI_MAPREG_MEM64_ADDR(mr)					\
	    ((mr) & PCI_MAPREG_MEM64_ADDR_MASK)
#define	PCI_MAPREG_MEM64_SIZE(mr)					\
	    (PCI_MAPREG_MEM64_ADDR(mr) & -PCI_MAPREG_MEM64_ADDR(mr))
#define	PCI_MAPREG_MEM64_ADDR_MASK		0xfffffffffffffff0

#define	PCI_MAPREG_IO_ADDR(mr)						\
	    ((mr) & PCI_MAPREG_IO_ADDR_MASK)
#define	PCI_MAPREG_IO_SIZE(mr)						\
	    (PCI_MAPREG_IO_ADDR(mr) & -PCI_MAPREG_IO_ADDR(mr))
#define	PCI_MAPREG_IO_ADDR_MASK			0xfffffff8

#define	PCI_MAPREG_ROM_ADDR(mr)						\
	    ((mr) & PCI_MAPREG_ROM_ADDR_MASK)
#define	PCI_MAPREG_ROM_SIZE(mr)						\
	    (PCI_MAPREG_ROM_ADDR(mr) & -PCI_MAPREG_ROM_ADDR(mr))
#define	PCI_MAPREG_ROM_ADDR_MASK		0xfffff800

/*
 * Cardbus CIS pointer (PCI rev. 2.1)
 */
#define PCI_CARDBUS_CIS_REG 0x28

/*
 * Subsystem identification register; contains a vendor ID and a device ID.
 * Types/macros for PCI_ID_REG apply.
 * (PCI rev. 2.1)
 */
#define PCI_SUBSYS_ID_REG 0x2c

/*
 * capabilities link list (PCI rev. 2.2)
 */
#define PCI_CAPLISTPTR_REG		0x34
#define PCI_CAPLIST_PTR(cpr) ((cpr) & 0xff)
#define PCI_CAPLIST_NEXT(cr) (((cr) >> 8) & 0xff)
#define PCI_CAPLIST_CAP(cr) ((cr) & 0xff)

#define PCI_CAP_REESSERVED	0x00
#define PCI_CAP_PWRMGMT		0x01
#define PCI_CAP_AGP		0x02
#define PCI_CAP_VPD		0x03
#define PCI_CAP_SLOTID		0x04
#define PCI_CAP_MBI		0x05
#define PCI_CAP_CPCI_HOTSWAP	0x06
#define PCI_CAP_PCIX		0x07
#define PCI_CAP_LDT		0x08
#define PCI_CAP_VENDSPEC	0x09
#define PCI_CAP_DEBUGPORT	0x0a
#define PCI_CAP_CPCI_RSRCCTL	0x0b
#define PCI_CAP_HOTPLUG		0x0c

/*
 * Power Management Control Status Register; access via capability pointer.
 */
#define PCI_PMCSR_STATE_MASK	0x03
#define PCI_PMCSR_STATE_D0	0x00
#define PCI_PMCSR_STATE_D1	0x01
#define PCI_PMCSR_STATE_D2	0x02
#define PCI_PMCSR_STATE_D3	0x03

/*
 * Interrupt Configuration Register; contains interrupt pin and line.
 */
#define	PCI_INTERRUPT_REG		0x3c

typedef u_int8_t pci_intr_pin_t;
typedef u_int8_t pci_intr_line_t;

#define	PCI_INTERRUPT_PIN_SHIFT			8
#define	PCI_INTERRUPT_PIN_MASK			0xff
#define	PCI_INTERRUPT_PIN(icr) \
	    (((icr) >> PCI_INTERRUPT_PIN_SHIFT) & PCI_INTERRUPT_PIN_MASK)

#define	PCI_INTERRUPT_LINE_SHIFT		0
#define	PCI_INTERRUPT_LINE_MASK			0xff
#define	PCI_INTERRUPT_LINE(icr) \
	    (((icr) >> PCI_INTERRUPT_LINE_SHIFT) & PCI_INTERRUPT_LINE_MASK)

#define	PCI_MIN_GNT_SHIFT			16
#define	PCI_MIN_GNT_MASK			0xff
#define	PCI_MIN_GNT(icr) \
	    (((icr) >> PCI_MIN_GNT_SHIFT) & PCI_MIN_GNT_MASK)

#define	PCI_MAX_LAT_SHIFT			24
#define	PCI_MAX_LAT_MASK			0xff
#define	PCI_MAX_LAT(icr) \
	    (((icr) >> PCI_MAX_LAT_SHIFT) & PCI_MAX_LAT_MASK)

#define	PCI_INTERRUPT_PIN_NONE			0x00
#define	PCI_INTERRUPT_PIN_A			0x01
#define	PCI_INTERRUPT_PIN_B			0x02
#define	PCI_INTERRUPT_PIN_C			0x03
#define	PCI_INTERRUPT_PIN_D			0x04
#define	PCI_INTERRUPT_PIN_MAX			0x04

/* Capability lists */

#define PCI_CAP_LIST_ID		0	/* Capability ID */
#define  PCI_CAP_ID_PM		0x01	/* Power Management */
#define  PCI_CAP_ID_AGP		0x02	/* Accelerated Graphics Port */
#define  PCI_CAP_ID_VPD		0x03	/* Vital Product Data */
#define  PCI_CAP_ID_SLOTID	0x04	/* Slot Identification */
#define  PCI_CAP_ID_MSI		0x05	/* Message Signalled Interrupts */
#define  PCI_CAP_ID_CHSWP	0x06	/* CompactPCI HotSwap */
#define  PCI_CAP_ID_PCIX	0x07	/* PCI-X */
#define  PCI_CAP_ID_HT		0x08	/* HyperTransport */
#define  PCI_CAP_ID_VNDR	0x09	/* Vendor-Specific */
#define  PCI_CAP_ID_DBG		0x0A	/* Debug port */
#define  PCI_CAP_ID_CCRC	0x0B	/* CompactPCI Central Resource Control */
#define  PCI_CAP_ID_SHPC	0x0C	/* PCI Standard Hot-Plug Controller */
#define  PCI_CAP_ID_SSVID	0x0D	/* Bridge subsystem vendor/device ID */
#define  PCI_CAP_ID_AGP3	0x0E	/* AGP Target PCI-PCI bridge */
#define  PCI_CAP_ID_SECDEV	0x0F	/* Secure Device */
#define  PCI_CAP_ID_EXP		0x10	/* PCI Express */
#define  PCI_CAP_ID_MSIX	0x11	/* MSI-X */
#define  PCI_CAP_ID_SATA	0x12	/* SATA Data/Index Conf. */
#define  PCI_CAP_ID_AF		0x13	/* PCI Advanced Features */
#define  PCI_CAP_ID_MAX		PCI_CAP_ID_AF
#define PCI_CAP_LIST_NEXT	1	/* Next capability in the list */
#define PCI_CAP_FLAGS		2	/* Capability defined flags (16 bits) */
#define PCI_CAP_SIZEOF		4

/* PCI Express capability registers */

#define PCI_EXP_FLAGS		2	/* Capabilities register */
#define PCI_EXP_FLAGS_VERS	0x000f	/* Capability version */
#define PCI_EXP_FLAGS_TYPE	0x00f0	/* Device/Port type */
#define  PCI_EXP_TYPE_ENDPOINT	0x0	/* Express Endpoint */
#define  PCI_EXP_TYPE_LEG_END	0x1	/* Legacy Endpoint */
#define  PCI_EXP_TYPE_ROOT_PORT 0x4	/* Root Port */
#define  PCI_EXP_TYPE_UPSTREAM	0x5	/* Upstream Port */
#define  PCI_EXP_TYPE_DOWNSTREAM 0x6	/* Downstream Port */
#define  PCI_EXP_TYPE_PCI_BRIDGE 0x7	/* PCIe to PCI/PCI-X Bridge */
#define  PCI_EXP_TYPE_PCIE_BRIDGE 0x8	/* PCI/PCI-X to PCIe Bridge */
#define  PCI_EXP_TYPE_RC_END	0x9	/* Root Complex Integrated Endpoint */
#define  PCI_EXP_TYPE_RC_EC	0xa	/* Root Complex Event Collector */
#define PCI_EXP_FLAGS_SLOT	0x0100	/* Slot implemented */
#define PCI_EXP_FLAGS_IRQ	0x3e00	/* Interrupt message number */
#define PCI_EXP_DEVCAP		4	/* Device capabilities */
#define  PCI_EXP_DEVCAP_PAYLOAD	0x00000007 /* Max_Payload_Size */
#define  PCI_EXP_DEVCAP_PHANTOM	0x00000018 /* Phantom functions */
#define  PCI_EXP_DEVCAP_EXT_TAG	0x00000020 /* Extended tags */
#define  PCI_EXP_DEVCAP_L0S	0x000001c0 /* L0s Acceptable Latency */
#define  PCI_EXP_DEVCAP_L1	0x00000e00 /* L1 Acceptable Latency */
#define  PCI_EXP_DEVCAP_ATN_BUT	0x00001000 /* Attention Button Present */
#define  PCI_EXP_DEVCAP_ATN_IND	0x00002000 /* Attention Indicator Present */
#define  PCI_EXP_DEVCAP_PWR_IND	0x00004000 /* Power Indicator Present */
#define  PCI_EXP_DEVCAP_RBER	0x00008000 /* Role-Based Error Reporting */
#define  PCI_EXP_DEVCAP_PWR_VAL	0x03fc0000 /* Slot Power Limit Value */
#define  PCI_EXP_DEVCAP_PWR_SCL	0x0c000000 /* Slot Power Limit Scale */
#define  PCI_EXP_DEVCAP_FLR     0x10000000 /* Function Level Reset */
#define PCI_EXP_DEVCTL		8	/* Device Control */
#define  PCI_EXP_DEVCTL_CERE	0x0001	/* Correctable Error Reporting En. */
#define  PCI_EXP_DEVCTL_NFERE	0x0002	/* Non-Fatal Error Reporting Enable */
#define  PCI_EXP_DEVCTL_FERE	0x0004	/* Fatal Error Reporting Enable */
#define  PCI_EXP_DEVCTL_URRE	0x0008	/* Unsupported Request Reporting En. */
#define  PCI_EXP_DEVCTL_RELAX_EN 0x0010 /* Enable relaxed ordering */
#define  PCI_EXP_DEVCTL_PAYLOAD	0x00e0	/* Max_Payload_Size */
#define  PCI_EXP_DEVCTL_EXT_TAG	0x0100	/* Extended Tag Field Enable */
#define  PCI_EXP_DEVCTL_PHANTOM	0x0200	/* Phantom Functions Enable */
#define  PCI_EXP_DEVCTL_AUX_PME	0x0400	/* Auxiliary Power PM Enable */
#define  PCI_EXP_DEVCTL_NOSNOOP_EN 0x0800  /* Enable No Snoop */
#define  PCI_EXP_DEVCTL_READRQ	0x7000	/* Max_Read_Request_Size */
#define  PCI_EXP_DEVCTL_READRQ_128B  0x0000 /* 128 Bytes */
#define  PCI_EXP_DEVCTL_READRQ_256B  0x1000 /* 256 Bytes */
#define  PCI_EXP_DEVCTL_READRQ_512B  0x2000 /* 512 Bytes */
#define  PCI_EXP_DEVCTL_READRQ_1024B 0x3000 /* 1024 Bytes */
#define  PCI_EXP_DEVCTL_BCR_FLR 0x8000  /* Bridge Configuration Retry / FLR */
#define PCI_EXP_DEVSTA		10	/* Device Status */
#define  PCI_EXP_DEVSTA_CED	0x0001	/* Correctable Error Detected */
#define  PCI_EXP_DEVSTA_NFED	0x0002	/* Non-Fatal Error Detected */
#define  PCI_EXP_DEVSTA_FED	0x0004	/* Fatal Error Detected */
#define  PCI_EXP_DEVSTA_URD	0x0008	/* Unsupported Request Detected */
#define  PCI_EXP_DEVSTA_AUXPD	0x0010	/* AUX Power Detected */
#define  PCI_EXP_DEVSTA_TRPND	0x0020	/* Transactions Pending */
#define PCI_EXP_LNKCAP		12	/* Link Capabilities */
#define  PCI_EXP_LNKCAP_SLS	0x0000000f /* Supported Link Speeds */
#define  PCI_EXP_LNKCAP_SLS_2_5GB 0x00000001 /* LNKCAP2 SLS Vector bit 0 */
#define  PCI_EXP_LNKCAP_SLS_5_0GB 0x00000002 /* LNKCAP2 SLS Vector bit 1 */
#define  PCI_EXP_LNKCAP_MLW	0x000003f0 /* Maximum Link Width */
#define  PCI_EXP_LNKCAP_ASPMS	0x00000c00 /* ASPM Support */
#define  PCI_EXP_LNKCAP_L0SEL	0x00007000 /* L0s Exit Latency */
#define  PCI_EXP_LNKCAP_L1EL	0x00038000 /* L1 Exit Latency */
#define  PCI_EXP_LNKCAP_CLKPM	0x00040000 /* Clock Power Management */
#define  PCI_EXP_LNKCAP_SDERC	0x00080000 /* Surprise Down Error Reporting Capable */
#define  PCI_EXP_LNKCAP_DLLLARC	0x00100000 /* Data Link Layer Link Active Reporting Capable */
#define  PCI_EXP_LNKCAP_LBNC	0x00200000 /* Link Bandwidth Notification Capability */
#define  PCI_EXP_LNKCAP_PN	0xff000000 /* Port Number */
#define PCI_EXP_LNKCTL		16	/* Link Control */
#define  PCI_EXP_LNKCTL_ASPMC	0x0003	/* ASPM Control */
#define  PCI_EXP_LNKCTL_ASPM_L0S 0x0001	/* L0s Enable */
#define  PCI_EXP_LNKCTL_ASPM_L1  0x0002	/* L1 Enable */
#define  PCI_EXP_LNKCTL_RCB	0x0008	/* Read Completion Boundary */
#define  PCI_EXP_LNKCTL_LD	0x0010	/* Link Disable */
#define  PCI_EXP_LNKCTL_RL	0x0020	/* Retrain Link */
#define  PCI_EXP_LNKCTL_CCC	0x0040	/* Common Clock Configuration */
#define  PCI_EXP_LNKCTL_ES	0x0080	/* Extended Synch */
#define  PCI_EXP_LNKCTL_CLKREQ_EN 0x0100 /* Enable clkreq */
#define  PCI_EXP_LNKCTL_HAWD	0x0200	/* Hardware Autonomous Width Disable */
#define  PCI_EXP_LNKCTL_LBMIE	0x0400	/* Link Bandwidth Management Interrupt Enable */
#define  PCI_EXP_LNKCTL_LABIE	0x0800	/* Link Autonomous Bandwidth Interrupt Enable */
#define PCI_EXP_LNKSTA		18	/* Link Status */
#define  PCI_EXP_LNKSTA_CLS	0x000f	/* Current Link Speed */
#define  PCI_EXP_LNKSTA_CLS_2_5GB 0x0001 /* Current Link Speed 2.5GT/s */
#define  PCI_EXP_LNKSTA_CLS_5_0GB 0x0002 /* Current Link Speed 5.0GT/s */
#define  PCI_EXP_LNKSTA_CLS_8_0GB 0x0003 /* Current Link Speed 8.0GT/s */
#define  PCI_EXP_LNKSTA_NLW	0x03f0	/* Negotiated Link Width */
#define  PCI_EXP_LNKSTA_NLW_X1	0x0010	/* Current Link Width x1 */
#define  PCI_EXP_LNKSTA_NLW_X2	0x0020	/* Current Link Width x2 */
#define  PCI_EXP_LNKSTA_NLW_X4	0x0040	/* Current Link Width x4 */
#define  PCI_EXP_LNKSTA_NLW_X8	0x0080	/* Current Link Width x8 */
#define  PCI_EXP_LNKSTA_NLW_SHIFT 4	/* start of NLW mask in link status */
#define  PCI_EXP_LNKSTA_LT	0x0800	/* Link Training */
#define  PCI_EXP_LNKSTA_SLC	0x1000	/* Slot Clock Configuration */
#define  PCI_EXP_LNKSTA_DLLLA	0x2000	/* Data Link Layer Link Active */
#define  PCI_EXP_LNKSTA_LBMS	0x4000	/* Link Bandwidth Management Status */
#define  PCI_EXP_LNKSTA_LABS	0x8000	/* Link Autonomous Bandwidth Status */
#define PCI_CAP_EXP_ENDPOINT_SIZEOF_V1	20	/* v1 endpoints end here */
#define PCI_EXP_SLTCAP		20	/* Slot Capabilities */
#define  PCI_EXP_SLTCAP_ABP	0x00000001 /* Attention Button Present */
#define  PCI_EXP_SLTCAP_PCP	0x00000002 /* Power Controller Present */
#define  PCI_EXP_SLTCAP_MRLSP	0x00000004 /* MRL Sensor Present */
#define  PCI_EXP_SLTCAP_AIP	0x00000008 /* Attention Indicator Present */
#define  PCI_EXP_SLTCAP_PIP	0x00000010 /* Power Indicator Present */
#define  PCI_EXP_SLTCAP_HPS	0x00000020 /* Hot-Plug Surprise */
#define  PCI_EXP_SLTCAP_HPC	0x00000040 /* Hot-Plug Capable */
#define  PCI_EXP_SLTCAP_SPLV	0x00007f80 /* Slot Power Limit Value */
#define  PCI_EXP_SLTCAP_SPLS	0x00018000 /* Slot Power Limit Scale */
#define  PCI_EXP_SLTCAP_EIP	0x00020000 /* Electromechanical Interlock Present */
#define  PCI_EXP_SLTCAP_NCCS	0x00040000 /* No Command Completed Support */
#define  PCI_EXP_SLTCAP_PSN	0xfff80000 /* Physical Slot Number */
#define PCI_EXP_SLTCTL		24	/* Slot Control */
#define  PCI_EXP_SLTCTL_ABPE	0x0001	/* Attention Button Pressed Enable */
#define  PCI_EXP_SLTCTL_PFDE	0x0002	/* Power Fault Detected Enable */
#define  PCI_EXP_SLTCTL_MRLSCE	0x0004	/* MRL Sensor Changed Enable */
#define  PCI_EXP_SLTCTL_PDCE	0x0008	/* Presence Detect Changed Enable */
#define  PCI_EXP_SLTCTL_CCIE	0x0010	/* Command Completed Interrupt Enable */
#define  PCI_EXP_SLTCTL_HPIE	0x0020	/* Hot-Plug Interrupt Enable */
#define  PCI_EXP_SLTCTL_AIC	0x00c0	/* Attention Indicator Control */
#define  PCI_EXP_SLTCTL_ATTN_IND_ON    0x0040 /* Attention Indicator on */
#define  PCI_EXP_SLTCTL_ATTN_IND_BLINK 0x0080 /* Attention Indicator blinking */
#define  PCI_EXP_SLTCTL_ATTN_IND_OFF   0x00c0 /* Attention Indicator off */
#define  PCI_EXP_SLTCTL_PIC	0x0300	/* Power Indicator Control */
#define  PCI_EXP_SLTCTL_PWR_IND_ON     0x0100 /* Power Indicator on */
#define  PCI_EXP_SLTCTL_PWR_IND_BLINK  0x0200 /* Power Indicator blinking */
#define  PCI_EXP_SLTCTL_PWR_IND_OFF    0x0300 /* Power Indicator off */
#define  PCI_EXP_SLTCTL_PCC	0x0400	/* Power Controller Control */
#define  PCI_EXP_SLTCTL_PWR_ON         0x0000 /* Power On */
#define  PCI_EXP_SLTCTL_PWR_OFF        0x0400 /* Power Off */
#define  PCI_EXP_SLTCTL_EIC	0x0800	/* Electromechanical Interlock Control */
#define  PCI_EXP_SLTCTL_DLLSCE	0x1000	/* Data Link Layer State Changed Enable */
#define PCI_EXP_SLTSTA		26	/* Slot Status */
#define  PCI_EXP_SLTSTA_ABP	0x0001	/* Attention Button Pressed */
#define  PCI_EXP_SLTSTA_PFD	0x0002	/* Power Fault Detected */
#define  PCI_EXP_SLTSTA_MRLSC	0x0004	/* MRL Sensor Changed */
#define  PCI_EXP_SLTSTA_PDC	0x0008	/* Presence Detect Changed */
#define  PCI_EXP_SLTSTA_CC	0x0010	/* Command Completed */
#define  PCI_EXP_SLTSTA_MRLSS	0x0020	/* MRL Sensor State */
#define  PCI_EXP_SLTSTA_PDS	0x0040	/* Presence Detect State */
#define  PCI_EXP_SLTSTA_EIS	0x0080	/* Electromechanical Interlock Status */
#define  PCI_EXP_SLTSTA_DLLSC	0x0100	/* Data Link Layer State Changed */
#define PCI_EXP_RTCTL		28	/* Root Control */
#define  PCI_EXP_RTCTL_SECEE	0x0001	/* System Error on Correctable Error */
#define  PCI_EXP_RTCTL_SENFEE	0x0002	/* System Error on Non-Fatal Error */
#define  PCI_EXP_RTCTL_SEFEE	0x0004	/* System Error on Fatal Error */
#define  PCI_EXP_RTCTL_PMEIE	0x0008	/* PME Interrupt Enable */
#define  PCI_EXP_RTCTL_CRSSVE	0x0010	/* CRS Software Visibility Enable */
#define PCI_EXP_RTCAP		30	/* Root Capabilities */
#define  PCI_EXP_RTCAP_CRSVIS	0x0001	/* CRS Software Visibility capability */
#define PCI_EXP_RTSTA		32	/* Root Status */
#define PCI_EXP_RTSTA_PME	0x00010000 /* PME status */
#define PCI_EXP_RTSTA_PENDING	0x00020000 /* PME pending */

#endif /* _DEV_PCI_PCIREG_H_ */
