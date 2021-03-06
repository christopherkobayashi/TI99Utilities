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

#pragma ident "@(#)intl.h	1.2	01/01/01 meem"

#ifndef	INTL_H
#define	INTL_H

#include <locale.h>

#ifdef	ENABLE_NLS

#include <libintl.h>
#define	_(string)	gettext(string)
#define	N_(string)	gettext_noop(string)

#else

/*
 * No NLS -- stub out the functions.
 */

#define	_(string)	string
#define	N_(string)	string
#define	bindtextdomain(pkg, localedir)
#define	textdomain(pkg)

#endif

#endif	/* INTL_H */
