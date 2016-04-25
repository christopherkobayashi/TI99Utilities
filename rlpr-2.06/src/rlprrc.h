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

#pragma ident "@(#)rlprrc.h	1.2	99/09/25 meem"

#ifndef	RLPRRC_H
#define	RLPRCC_H

typedef enum { R_RESOLVE_HOST, R_RESOLVE_QUEUE } rlpr_search_type_e;

#define	R_LINE_LEN		1024
#define	R_RLPRRC_NAME		"rlprrc"
#define	R_CONFDIR		"/etc"

const char *	rlprrc(const char *, rlpr_search_type_e);

#endif /* RLPRRC_H */
