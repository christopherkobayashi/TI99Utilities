/*
 * Copyright (c) 1998 peter memishian (meem), meem@gnu.org
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

#pragma ident "@(#)rlprd.h	1.1	99/09/16 meem"

#ifndef	RLPRD_H
#define	RLPRD_H

#include <sys/types.h>
#include "component.h"

#define	R_RLPRD_LISTEN_PORT	7290

struct rlpr_rlprd
{
    int		no_daemon;	/* whether rlprd acts as a daemon (default) */
    u_short	listen_port;	/* port number that rlprd listens on */
    int		timeout;	/* inactivity timeout */
};

extern struct component comp_rlprd;

#endif /* RLPRD_H */
