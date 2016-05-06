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

#pragma ident "@(#)rlpq.h	1.1	99/09/16 meem"

#ifndef	RLPQ_H
#define	RLPQ_H

#include <sys/types.h>
#include "component.h"

struct rlpr_rlpq
{
    int		long_output;	/* produce verbose output */
    int		mine_only;	/* list only the current user's jobs */
    int		timeout;	/* inactivity timeout */
    char       *user;		/* the user who invoked rlpq */
};

extern struct component comp_rlpq;

#endif /* RLPQ_H */
