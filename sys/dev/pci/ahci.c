/*
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *		Dave Liu <daveliu@freescale.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include "bpfilter.h"
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <autoconf.h>

#include <ctype.h>

#if defined(__NetBSD__) || defined(__OpenBSD__)

#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/device.h>

#include <vm/vm.h>

#include <machine/cpu.h>
#include <machine/bus.h>
#include <machine/intr.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>

#include <dev/pci/if_fxpreg.h>

#else /* __FreeBSD__ */

#include <sys/sockio.h>

#include <machine/clock.h>	/* for DELAY */

#include <pci/pcivar.h>

#endif /* __NetBSD__ || __OpenBSD__ */

#if NGZIP > 0
#include <gzipfs.h>
#endif /* NGZIP */

#include <machine/intr.h>
#include <machine/bus.h>

#include <dev/ata/atareg.h>
#include <dev/ic/wdcreg.h>

#include <linux/libata.h>
#include <fis.h>
#include <part.h>
#include <pmon.h>
#include "ahci.h"
#include "ahcisata.h"
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
#include "include/ls1a.h"
#endif

/* Maximum timeouts for each event */
#define WAIT_MS_SPINUP	20000
#define WAIT_MS_DATAIO	10000
#define WAIT_MS_FLUSH	5000
#define WAIT_MS_LINKUP	200
#define debug printf

int ahci_host_init(struct ahci_probe_ent *probe_ent);
static void *ahci_init_one(u32 regbase);

static int ahci_match(struct device *, void *, void *);
static void ahci_attach(struct device *, struct device *, void *);

static inline u32 ahci_port_base(u32 base, u32 port)
{
	return base + 0x100 + (port * 0x80);
}

static void ahci_setup_port(struct ahci_ioports *port, unsigned long base,
			    unsigned int port_idx)
{
	base = ahci_port_base(base, port_idx);

	port->cmd_addr = base;
	port->scr_addr = base + PORT_SCR;
}

struct cfattach ahci_ca = {
	sizeof(ahci_sata_t), ahci_match, ahci_attach,
};

struct cfdriver ahci_cd = {
	NULL, "ahci", DV_DULL
};

static int ahci_match(struct device *parent, void *match, void *aux)
{
	struct pci_attach_args *pa = aux;

	if((PCI_VENDOR(pa->pa_id) == PCI_VENDOR_SATA && PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_SATA) ||
		(PCI_VENDOR(pa->pa_id) == PCI_VENDOR_2KSATA && PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_2KSATA) ||
		(PCI_VENDOR(pa->pa_id) == 0x1B4B && PCI_PRODUCT(pa->pa_id) == 0x9215) ||
		(pa->pa_class >> 8) == 0x010601)
		return 1;

	return 0;
}

static void ahci_attach(struct device *parent, struct device *self, void *aux)
{
	struct pci_attach_args *pa = aux;
	struct pci_chipset_tag_t *pc = pa->pa_pc;
	bus_space_tag_t memt = pa->pa_memt;
	bus_addr_t membasep;
	bus_size_t memsizep;
	int i;
	u32 linkmap;
	ahci_sata_info_t info;
	struct ahci_probe_ent *probe_ent;

	if((PCI_VENDOR(pa->pa_id) == PCI_VENDOR_2KSATA && PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_2KSATA))
	{
		if (pci_mem_find(NULL, pa->pa_tag, 0x10, &membasep, &memsizep, NULL)) {
			printf(" Can't find mem space\n");
		return;
		}
	} else if (pci_mem_find(NULL, pa->pa_tag, 0x24, &membasep, &memsizep, NULL)) {
		printf(" Can't find mem space\n");
		return;
	}
	printf("Found memory space: memt->bus_base=0x%x, baseaddr=0x%x"
	       "size=0x%x\n", memt->bus_base, (u32) (membasep),
	       (u32) (memsizep));

#if 0 /* set 1.0 mode */
	int temp = *(int *)((membasep + 0x12c) | memt->bus_base);
	printf("0x12C =%x\n", temp);
	*(int *)((membasep + 0x12c) | memt->bus_base) = temp & 0xffffff00 | 0x11;

	temp = *(int *)((membasep + 0x12c) | memt->bus_base);
	printf("0x12C =%x\n", temp);
#endif

	if (!(probe_ent = ahci_init_one((u32) (memt->bus_base | (u32) (membasep))))) {
		printf("ahci_init_one failed.\n");
	}

	linkmap = probe_ent->link_port_map;
	printf("ahci: linkmap=%x\n", linkmap);
	for (i = 0; i < probe_ent->n_ports; i++) {
		if (((linkmap >> i) & 0x01)) {
			info.sata_reg_base =
			    memt->bus_base | (u32) (membasep) + 0x100 + i * 0x80;
			info.flags = i;
			info.aa_link.aa_type = 0xff;	/* just for not match ide */
			info.probe_ent = probe_ent;
			config_found(self, (void *)&info, NULL);
		}
	}

}

static int lahci_match(struct device *parent, void *match, void *aux)
{
	return 1;
}

static void lahci_attach(struct device *parent, struct device *self, void *aux)
{
	struct confargs *cf = aux;
	bus_space_handle_t regbase;
	int i;
	u32 linkmap;
	ahci_sata_info_t info;
	struct ahci_probe_ent *probe_ent;
#if defined(LOONGSON_2G1A) || defined(LOONGSON_2F1A)
	*(volatile int *)LS1A_SATA_CLK_REG  = 0x38682650; //100MHZ
#endif
	regbase = (bus_space_handle_t) cf->ca_baseaddr;;
	printf("%s:%d: regbase = %08x\n", __FUNCTION__, __LINE__, regbase);
	if (!(probe_ent = ahci_init_one(regbase))) {
		printf("ahci_init_one failed.\n");
	}


	linkmap = probe_ent->link_port_map;
	printf("lahci: linkmap=%x\n", linkmap);
	for (i = 0; i < 2; i++) {
		if (((linkmap >> i) & 0x01)) {
			info.sata_reg_base = regbase + 0x100 + i * 0x80;
			info.flags = i;
			info.aa_link.aa_type = 0xff;	/* just for not match ide */
			config_found(self, (void *)&info, NULL);
		}
	}
}

struct cfattach lahci_ca = {
	sizeof(ahci_sata_t), lahci_match, lahci_attach,
};

struct cfdriver lahci_cd = {
	NULL, "ahci", DV_DULL
};

static void ahci_enable_ahci(void *mmio)
{
	int i;
	u32 tmp;

	/* turn on AHCI_EN */
	tmp = readl(mmio + HOST_CTL);
	if (tmp & HOST_AHCI_EN)
		return;

	/* Some controllers need AHCI_EN to be written multiple times.
	 * Try a few times before giving up.
	 */
	for (i = 0; i < 5; i++) {
		tmp |= HOST_AHCI_EN;
		writel(tmp, mmio + HOST_CTL);
		tmp = readl(mmio + HOST_CTL);	/* flush && sanity check */
		if (tmp & HOST_AHCI_EN)
			return;
		msleep(10);
	}
}

int  ahci_link_up(struct ahci_probe_ent *probe_ent, u8 port)
{
	u32 tmp;
	int j = 0;
	void *port_mmio = probe_ent->port[port].port_mmio;

	/*
	 * Bring up SATA link.
	 * SATA link bringup time is usually less than 1 ms; only very
	 * rarely has it taken between 1-2 ms. Never seen it above 2 ms.
	 */
	while (j < WAIT_MS_LINKUP) {
		tmp = readl(port_mmio + PORT_SCR_STAT);
		tmp &= PORT_SCR_STAT_DET_MASK;
		if (tmp == PORT_SCR_STAT_DET_PHYRDY)
			return 0;
		udelay(1000);
		j++;
	}
	return 1;
}

int ahci_reset(void  *base)
{
	int i = 1000;
	u32 *host_ctl_reg = base + HOST_CTL;
	u32 tmp = readl(host_ctl_reg); /* global controller reset */

	if ((tmp & HOST_RESET) == 0)
		writel_with_flush(tmp | HOST_RESET, host_ctl_reg);

	/*
	 * reset must complete within 1 second, or
	 * the hardware should be considered fried.
	 */
	do {
		udelay(1000);
		tmp = readl(host_ctl_reg);
		i--;
	} while ((i > 0) && (tmp & HOST_RESET));

	if (i == 0) {
		printf("controller reset failed (0x%x)\n", tmp);
		return -1;
	}

	return 0;
}

int ahci_host_init(struct ahci_probe_ent *probe_ent)
{
	volatile u8 *mmio = (volatile u8 *)probe_ent->mmio_base;
	u32 tmp, cap_save, cmd;
	int i, j, ret;
	void  *port_mmio;
	u32 port_map;

	debug("ahci_host_init: start\n");

	cap_save = readl(mmio + HOST_CAP);
	cap_save &= ((1 << 28) | (1 << 17));
	cap_save |= (1 << 27);  /* Staggered Spin-up. Not needed. */

	ret = ahci_reset(probe_ent->mmio_base);
	if (ret)
		return ret;

	writel_with_flush(HOST_AHCI_EN, mmio + HOST_CTL);
	writel(cap_save, mmio + HOST_CAP);
	writel_with_flush(0xf, mmio + HOST_PORTS_IMPL);

	probe_ent->cap = readl(mmio + HOST_CAP);
	probe_ent->port_map = readl(mmio + HOST_PORTS_IMPL);
	port_map = probe_ent->port_map;
#ifndef AHCI_PORT
	probe_ent->n_ports = (probe_ent->cap & 0x1f) + 1;
	i = 0;
#else
	i= AHCI_PORT;
	probe_ent->n_ports = AHCI_PORT + 1;
#endif

	debug("cap 0x%x  port_map 0x%x  n_ports %d\n",
	      probe_ent->cap, probe_ent->port_map, probe_ent->n_ports);


	for (; i < probe_ent->n_ports; i++) {
		if (!(port_map & (1 << i)))
			continue;
		probe_ent->port[i].port_mmio = ahci_port_base(mmio, i);
		port_mmio = (u8 *)probe_ent->port[i].port_mmio;

		/* make sure port is not active */
		tmp = readl(port_mmio + PORT_CMD);
		if (tmp & (PORT_CMD_LIST_ON | PORT_CMD_FIS_ON |
			   PORT_CMD_FIS_RX | PORT_CMD_START)) {
			debug("Port %d is active. Deactivating.\n", i);
			tmp &= ~(PORT_CMD_LIST_ON | PORT_CMD_FIS_ON |
				 PORT_CMD_FIS_RX | PORT_CMD_START);
			writel_with_flush(tmp, port_mmio + PORT_CMD);

			/* spec says 500 msecs for each bit, so
			 * this is slightly incorrect.
			 */
			msleep(500);
		}


		/* Add the spinup command to whatever mode bits may
		 * already be on in the command register.
		 */
		cmd = readl(port_mmio + PORT_CMD);
		cmd |= PORT_CMD_SPIN_UP;
		writel_with_flush(cmd, port_mmio + PORT_CMD);

		/* Bring up SATA link. */
		ret = ahci_link_up(probe_ent, i);
		if (ret) {
			printf("SATA link %d timeout.\n", i);
			continue;
		} else {
			debug("SATA link ok.\n");
		}

		/* Clear error status */
		tmp = readl(port_mmio + PORT_SCR_ERR);
		if (tmp)
			writel(tmp, port_mmio + PORT_SCR_ERR);

		debug("Spinning up device on SATA port %d... ", i);

		j = 0;
		while (j < WAIT_MS_SPINUP) {
			tmp = readl(port_mmio + PORT_TFDATA);
			if (!(tmp & (ATA_BUSY | ATA_DRQ)))
				break;
			udelay(1000);
			tmp = readl(port_mmio + PORT_SCR_STAT);
			tmp &= PORT_SCR_STAT_DET_MASK;
			if (tmp == PORT_SCR_STAT_DET_PHYRDY)
				break;
			j++;
		}

		tmp = readl(port_mmio + PORT_SCR_STAT) & PORT_SCR_STAT_DET_MASK;
		if (tmp == PORT_SCR_STAT_DET_COMINIT) {
			debug("SATA link %d down (COMINIT received), retrying...\n", i);
			i--;
			continue;
		}

		printf("Target spinup took %d ms.\n", j);
		if (j == WAIT_MS_SPINUP)
			debug("timeout.\n");
		else
			debug("ok.\n");

		tmp = readl(port_mmio + PORT_SCR_ERR);
		debug("PORT_SCR_ERR 0x%x\n", tmp);
		writel(tmp, port_mmio + PORT_SCR_ERR);

		/* ack any pending irq events for this port */
		tmp = readl(port_mmio + PORT_IRQ_STAT);
		debug("PORT_IRQ_STAT 0x%x\n", tmp);
		if (tmp)
			writel(tmp, port_mmio + PORT_IRQ_STAT);

		writel(1 << i, mmio + HOST_IRQ_STAT);

		/* register linkup ports */
		tmp = readl(port_mmio + PORT_SCR_STAT);
		debug("SATA port %d status: 0x%x\n", i, tmp);
		if ((tmp & PORT_SCR_STAT_DET_MASK) == PORT_SCR_STAT_DET_PHYRDY)
			probe_ent->link_port_map |= (0x01 << i);
	}

	tmp = readl(mmio + HOST_CTL);
	debug("HOST_CTL 0x%x\n", tmp);
	writel(tmp | HOST_IRQ_EN, mmio + HOST_CTL);
	tmp = readl(mmio + HOST_CTL);
	debug("HOST_CTL 0x%x\n", tmp);
	return 0;
}

static void ahci_print_info(struct ahci_probe_ent *probe_ent)
{
	volatile u8 *mmio = (volatile u8 *)probe_ent->mmio_base;
	u32 vers, cap, impl, speed;
	const char *speed_s;
	const char *scc_s;

	vers = readl(mmio + HOST_VERSION);
	cap = probe_ent->cap;
	impl = probe_ent->port_map;

	speed = (cap >> 20) & 0xf;
	if (speed == 1)
		speed_s = "1.5";
	else if (speed == 2)
		speed_s = "3";
	else
		speed_s = "?";

	scc_s = "SATA";

	printf("AHCI %02x%02x.%02x%02x "
	       "%u slots %u ports %s Gbps 0x%x impl %s mode\n",
	       (vers >> 24) & 0xff,
	       (vers >> 16) & 0xff,
	       (vers >> 8) & 0xff,
	       vers & 0xff,
	       ((cap >> 8) & 0x1f) + 1, (cap & 0x1f) + 1, speed_s, impl, scc_s);

	printf("flags: "
	       "%s%s%s%s%s%s"
	       "%s%s%s%s%s%s%s\n",
	       cap & (1 << 31) ? "64bit " : "",
	       cap & (1 << 30) ? "ncq " : "",
	       cap & (1 << 28) ? "ilck " : "",
	       cap & (1 << 27) ? "stag " : "",
	       cap & (1 << 26) ? "pm " : "",
	       cap & (1 << 25) ? "led " : "",
	       cap & (1 << 24) ? "clo " : "",
	       cap & (1 << 19) ? "nz " : "",
	       cap & (1 << 18) ? "only " : "",
	       cap & (1 << 17) ? "pmp " : "",
	       cap & (1 << 15) ? "pio " : "",
	       cap & (1 << 14) ? "slum " : "", cap & (1 << 13) ? "part " : "");
}

static void *ahci_init_one(u32 regbase)
{
	int rc;
	struct ahci_probe_ent *probe_ent;

#if MY_MALLOC
	probe_ent = malloc(sizeof(struct ahci_probe_ent));
#else
	probe_ent = malloc(sizeof(struct ahci_probe_ent), M_DEVBUF, M_NOWAIT);
#endif
	memset(probe_ent, 0, sizeof(struct ahci_probe_ent));

	probe_ent->host_flags = ATA_FLAG_SATA
	    | ATA_FLAG_NO_LEGACY | ATA_FLAG_MMIO | ATA_FLAG_PIO_DMA;

	probe_ent->pio_mask = 0x1f;
	probe_ent->udma_mask = 0x7f;	/*Fixme,assume to support UDMA6 */

	probe_ent->mmio_base = regbase;
	printf("%s:%d: regbase = %08x\n", __FUNCTION__, __LINE__, regbase);

	/* initialize adapter */
	rc = ahci_host_init(probe_ent);
	if (rc)
		goto err_out;

	ahci_print_info(probe_ent);

	return probe_ent;

err_out:
	free(probe_ent, M_DEVBUF);
	return NULL;
}
