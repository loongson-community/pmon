/*
 * Copyright (C) 2009 Lemote, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author : liujl <liujl@lemote.com>
 * Date : 2009-08-18
 *
 * ht :
 * 	ht speed and width training routine
 */

/*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <sys/linux/types.h>
#include <include/bonito.h>
#include <machine/pio.h> 
#include <include/rs690.h>
#include <pci/rs690_cfg.h>
#include "rs690_config.h"

/*---------------------------------------------------------------------*/

/*
 * rs690_nb_ht_init :
 * RS690 NB spec. initialization for spec. HT speed.
 */
static void rs690_nb_ht_init(pcitag_t nb_dev)
{

	return;
}


/* 
 * rs690_ht_init :
 * Init HT link speed and width for M690 and loongson2G cpu
 */
void rs690_ht_init(void)
{
	pcitag_t nb_dev = _pci_make_tag(0, 0, 0);
	
	rs690_nb_ht_init(nb_dev);

	return;
}

/*--------------------------------------------------------------------*/

