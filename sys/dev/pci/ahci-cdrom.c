/*
 * Copyright (C) 2013 Loongson Technology Corporation Limited
 *	Shaozong Liu <liushaozong@loongson.cn>
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
#include <ctype.h>
#include <pmon.h>

#include <sys/buf.h>
#include <sys/unistd.h>
#include <sys/param.h>
#include <sys/device.h>
#include <linux/libata.h>

#include <machine/bus.h>

#include "ahcisata.h"
#include "ahci.h"
#include "ata.h"

#ifdef CD_DEBUG
#define cd_debug(fmt, args...) printf("%s<%d>: "fmt, __func__,	\
		__LINE__, ##args)
#else
#define cd_debug(fmt, arg...)
#endif

static int ahci_cdrom_match(struct device *parent, void *match, void *aux)
{
	ahci_sata_info_t *info;
	struct ahci_ioports *pp;
	volatile u8 *port_mmio;
	int rc;

	info = (ahci_sata_info_t *) aux;
	pp = &(probe_ent->port[info->flags]);
	port_mmio = (volatile u8 *)pp->port_mmio;

	rc = readl(port_mmio + PORT_SIG) >> 16;
	if (rc == 0xeb14) {
		pp->is_atapi = 1;
		return 1;
	} else {
		return 0;
	}
}

static void ahci_cdrom_attach(struct device *parent,
			      struct device *self, void *aux)
{
	int err;
	struct ahci_sata_softc *cdrom_soft = (struct ahci_sata_softc *)self;
	ahci_sata_info_t *info = (ahci_sata_info_t *) aux;

	cdrom_soft->port_no = info->flags;
	cdrom_soft->bs = 2048;
	cdrom_soft->count = -1;

	err = ahci_sata_initialize(info->sata_reg_base, info->flags, cdrom_soft);
	if (!err)
		ahci_do_softreset(info->flags, 0, 0);

	cd_debug("%s device: %p, devno:%d, portno:%d, name:%s\n",
			self->dv_xname, self, self->dv_unit,
			cdrom_soft->port_no, cdrom_soft->name);
}

struct cfattach ahci_cdrom_ca = {
	sizeof(struct ahci_sata_softc), ahci_cdrom_match, ahci_cdrom_attach,
};

struct cfdriver ahci_cdrom_cd = {
	NULL, "cd", DV_DISK
};

void ahci_cdrom_strategy(struct buf *bp)
{
	struct ahci_sata_softc *priv;
	priv = ahci_sata_lookup(ahci_cdrom_cd, bp->b_dev);
	if (!priv) {
		printf("%s<%d>: no found device\n", __func__, __LINE__);
		return;
	}

	cd_debug("bp = %p, priv = %p\n", bp, priv);
	ahci_sata_strategy(bp, priv);
}

int ahci_cdrom_open(dev_t dev, int flag, int fmt, struct proc *p)
{
	struct ahci_sata_softc *priv;
	int port_no, rc;

	priv = ahci_sata_lookup(ahci_cdrom_cd, dev);
	if (!priv)
		return -1;

	port_no = priv->port_no;

	rc = cd_prepare(port_no, 1);
	if (rc < 0)
		ahci_kick_engine(port_no, 1);

	rc = cd_test_unit_ready(port_no);
	if (rc < 0)
		ahci_kick_engine(port_no, 1);

	return 0;
}

int ahci_cdrom_close(dev_t dev, int flag, int fmt, struct proc *p)
{
	return 0;
}

int ahci_cdrom_read(dev_t dev, struct uio *uio, int flags)
{
	return physio(ahci_cdrom_strategy, NULL, dev, B_READ, minphys, uio);
}

int ahci_cdrom_write(dev_t dev, struct uio *uio, int flags)
{
	return (physio(ahci_cdrom_strategy, NULL, dev, B_WRITE, minphys, uio));
}
