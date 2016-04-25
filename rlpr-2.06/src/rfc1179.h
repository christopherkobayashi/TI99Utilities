/*
 * Copyright (c) 1998-1999 peter memishian (meem), meem@gnu.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 */

#pragma ident "@(#)rfc1179.h	1.1	99/09/16 meem"

#ifndef	RFC1179_H
#define	RFC1179_H

#include <sys/types.h>

/*
 * nothing in this header file should need to be changed.  these
 * constants are defined in rfc1179 and are not tunables.
 */

#define	R_LPD_SRC_PORT_LOW	721
#define	R_LPD_SRC_PORT_HIGH	731
#define	R_LPD_DST_PORT		515

enum { DONE, PRINTQ, RECVJ, SENDQS, SENDQL, REMJ };
enum { ABORTJ = 1, RECVCF, RECVDF };

#endif	/* RFC1179_H */
