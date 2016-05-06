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

#pragma ident "@(#)rlpr.h	1.1	99/09/16 meem"

#ifndef	RLPR_H
#define	RLPR_H

#include <sys/types.h>
#include "component.h"

struct rlpr_rlpr
{
    char	filetype;	/* type of file being printed */
    size_t	n_copies;	/* number of copies to print */

    /*
     * additional information regarding printed document
     */

    char       *class;		/* job classification on burst page */
    char       *job;		/* job name on burst page */
    char       *user;		/* user name on burst page */
    char       *title;		/* document title */

    /*
     * additional document characteristics
     */

    char       *width;		/* number of columns to indent by */
    char       *indentation;	/* width of printed page */

    /*
     * flags that set printing behavior
     */

    int		print_burst;	/* print burst page before printing */
    int		mail_after;	/* mail after printing */
    int		remove_after;	/* remove file after printing */
    int		windows_lpd;	/* operate in windows lpd mode */
    int		control_first;	/* send control file, then data file */

    /*
     * miscellany
     */

    char       *localhost;	/* our name */
    char       *hostname;	/* if nonzero, hostname to put control file */
    char       *tmpdir;		/* place to write temporary files */
    int		timeout;	/* inactivity timeout */

    /*
     * extensions to lpd protocol.  right now, these fields are only
     * used by --ext=hpux, but this may change in the future.
     */

    char       *options;	/* options to pass through to lpd */
    char       *priority;	/* priority of request */
    int		write_back;	/* write a message back to user when done */
};

#define	R_REQ_LEN	100
#define	R_TMPDIR	"/tmp/"

/*
 * these defaults are for compatibility with old bsd implementations
 */

#define	R_NOARG_INDENT	"8"
#define	R_NOARG_WIDTH	"80"

extern struct component comp_rlpr;

#endif /* RLPR_H */
