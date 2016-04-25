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

#pragma ident "@(#)msg.h	1.1	99/09/16 meem"

#ifndef	MSG_H
#define	MSG_H

#include <stdarg.h>
#include <getopt.h>
#include "component.h"

typedef enum
{
    R_DEBUG,			/* debugging message */
    R_INFO,			/* informational message */
    R_WARNING,			/* warning message */
    R_ERROR,			/* error message */
    R_FATAL,			/* fatal error message */
    R_STDOUT,
    R_TTY			/* free-form informational message if tty */

} rlpr_msg_level_e;

struct rlpr_msg
{
    char       *level[7];		/* level -> stringified level mapping */
    int		syslog_prio[7];		/* level -> syslog priority mapping */
    int		is_initialized;		/* has been initialized */
    int		is_debug;		/* debug mode on */
    int		is_quiet;		/* quiet mode on */
    int		is_verbose;		/* verbose mode on */
    int		is_compat;		/* bsd compatability on */
    int		use_syslog;		/* use syslog for output */
};

/* component-specific operations  */

void		msg(rlpr_msg_level_e, int, const char *, ...);
void		msg_use_syslog(int);
int		msg_is_quiet(void);

extern struct component comp_msg;

#endif	/* MSG_H */
