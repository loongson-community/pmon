/*********************************************************************
 *
 * $Id: theater_out.h,v 1.1.2.2 2004/01/27 22:50:35 fulivi Exp $
 *
 * Interface file for theater_out module
 *
 * Copyright (C) 2003 Federico Ulivi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * AUTHORS: F.Ulivi
 * NOTES:
 * $Log: theater_out.h,v $
 * Revision 1.1.2.2  2004/01/27 22:50:35  fulivi
 * Support for positioning/sizing of image added
 *
 * Revision 1.1.2.9  2004/01/18 23:01:12  fede
 * Functions for get/setting h/v pos/size replaced by
 * theaterOutSetAttr/theaterOutGetAttr/theaterOutGetAttrLimits
 * Circular inclusion with radeon.h fixed
 *
 * Revision 1.1.2.8  2004/01/11 21:43:32  fede
 * Fixed problem with definition of TVStd
 *
 * Revision 1.1.2.7  2004/01/05 00:09:59  fede
 * Functions for setting/getting H/V position added
 * Functions for setting/getting on/off attribute removed
 *
 * Revision 1.1.2.1  2003/11/26 19:50:10  fulivi
 * Support for ERT added
 *
 * Revision 1.1.2.6  2003/11/25 20:44:00  fede
 * TV_STD_KEEP_OFF added
 *
 * Revision 1.1.2.5  2003/10/14 18:41:32  fede
 * forceERT changed to forceVIP
 *
 * Revision 1.1.2.4  2003/10/11 12:30:30  fede
 * Support for ERT added
 *
 * Revision 1.1  2003/09/28 21:42:37  fulivi
 * Theater_out module added
 *
 * Revision 1.1.2.3  2003/09/28 15:26:09  fede
 * Minor aesthetic changes
 *
 * Revision 1.1.2.1  2003/08/31 13:36:35  fede
 * *** empty log message ***
 *
 *
 *********************************************************************/

#ifndef _THEATER_OUT_H
#define _THEATER_OUT_H

/**********************************************************************
 *
 * TheaterOutPtr
 *
 * Pointer to TheaterOut struct. Actual definition is in theater_out.c
 *
 **********************************************************************/
typedef struct TheaterOut *TheaterOutPtr;

/**********************************************************************
 *
 * TVStd
 *
 * Tv standard
 *
 **********************************************************************/
typedef enum {
	TV_STD_NTSC,
	TV_STD_PAL,
	TV_STD_PAL_M,
	TV_STD_PAL_60,
	TV_STD_NTSC_J,
	TV_STD_PAL_CN,
	TV_STD_PAL_N,
	TV_STD_KEEP_OFF,
	TV_STD_N_STANDARDS	/* Must be last */
} TVStd;

#endif				/* _THEATER_OUT_H */
