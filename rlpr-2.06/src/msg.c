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

#pragma ident "@(#)msg.c	1.3	04/09/07 meem"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <syslog.h>
#include <lib.h>		/* rlpr utility functions */

#include "intl.h"
#include "component.h"
#include "msg.h"

static struct rlpr_msg	*rlpr_msg;

static int
msg_init(void)
{
    struct rlpr_msg	rlpr_msg_tmpl = {

	{ "debug", "info", "warning", "error", "fatal error", 0, "tty"	   },
	{ LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERR, LOG_ERR, LOG_INFO, 0   }, 1

    };

    rlpr_msg	= xmalloc(sizeof (struct rlpr_msg));
    *rlpr_msg	= rlpr_msg_tmpl;

    return 1;
}

static int
msg_parse_args(int opt)
{
    const char	*invoked_name;

    /*
     * if we're being invoked as our bsd counterpart, act the role
     */

    if (bsd_program_name != 0) {
	invoked_name = strrchr(program_name, '/');
	invoked_name = invoked_name ? invoked_name + 1 : program_name;
	if (strcmp(invoked_name, bsd_program_name) == 0)
	    rlpr_msg->is_compat = 1;
    }

    switch (opt) {

    case EOF:

	if ((rlpr_msg->is_verbose || rlpr_msg->is_debug) &&
	    rlpr_msg->is_quiet) {

	    rlpr_msg->is_quiet = 0;
	    msg(R_WARNING, 0, "both `%s' and `quiet' selected.  turning "
		"off `quiet'.", rlpr_msg->is_debug ? "debug" : "verbose");
	}
	break;

    case -301:
	rlpr_msg->is_verbose = 1;
	break;

    case 'q':
	rlpr_msg->is_quiet = 1;
	break;

    case -300:
	rlpr_msg->is_debug = 1;
	break;
    }

    return 1;
}

static int
msg_fini(void)
{
    rlpr_msg->is_initialized = 0;
    free(rlpr_msg);

    return 1;
}

void
msg(rlpr_msg_level_e level, int errno, const char *format, ...)
{
    va_list		ap;
    FILE	       *stream = stderr;

    if (rlpr_msg == 0 || rlpr_msg->is_initialized == 0) {
	if (level > R_INFO)
	    fprintf(stderr, _("%s: msg() called but messaging system "
			      "inoperable\n"), program_name);
	return;
    }

    switch (level) {

    case R_DEBUG:

	if (!rlpr_msg->is_debug)
	    return;

	/* FALLTHRU into R_TTY */
    case R_TTY:
    case R_INFO:

	if (rlpr_msg->is_compat)
	    return;

	/* FALLTHRU into R_STDOUT */
    case R_STDOUT:

	stream = stdout;

	/* FALLTHRU into R_WARNING */
    case R_WARNING:

	if (rlpr_msg->is_quiet)
	    return;

	/* FALLTHRU into R_ERROR */
    case R_ERROR:
    case R_FATAL:

	break;
    }

    va_start(ap, format);

    if (level == R_TTY) {

	/*
	 * R_TTY only yields output if we're outputting from a
	 * terminal, and not reading from one, and we're not debugging
	 * (since debugging output would likely interfere with the
	 * uses of R_TTY)
	 */

	if (isatty(STDIN_FILENO) || isatty(STDOUT_FILENO) == 0 ||
	    rlpr_msg->is_debug)
	    return;

    } else if (rlpr_msg->use_syslog) {

	char 	   buf[1024];

	if (errno != 0) {
	    sprintf(buf, "%s: %s", _(format), strerror(errno));
	    format = buf;
	} else {
	    format = _(format);
	}

#ifdef HAVE_VSYSLOG
	vsyslog(rlpr_msg->syslog_prio[level], buf, ap);
#else
	{
	    char *a1 = va_arg(ap, char *);
	    char *a2 = va_arg(ap, char *);
	    char *a3 = va_arg(ap, char *);
	    char *a4 = va_arg(ap, char *);
	    char *a5 = va_arg(ap, char *);
	    char *a6 = va_arg(ap, char *);
	    char *a7 = va_arg(ap, char *);
	    char *a8 = va_arg(ap, char *);

	    syslog(rlpr_msg->syslog_prio[level], buf, a1, a2, a3, a4, a5, a6,
		a7, a8);
	}
#endif
    } else {

	fprintf(stream, "%s: ", program_name);
	if (rlpr_msg->level[level] != 0)
	    fprintf(stream, "%s: ", _(rlpr_msg->level[level]));

	vfprintf(stream, _(format), ap);
	fprintf(stream,	 errno ? ": %s\n" : "\n", strerror(errno));

	if (level == R_TTY)
	    fflush(stream);
    }

    va_end(ap);

    if (level == R_FATAL)
	exit(EXIT_FAILURE);
}

int
msg_is_quiet(void)
{
    return rlpr_msg->is_quiet;
}

void
msg_use_syslog(int yesno)
{
    if (yesno == 1)
	openlog(program_name, LOG_PID, LOG_DAEMON);
    else
	closelog();

    rlpr_msg->use_syslog = yesno;
}

static struct option msg_opts[] = {
    { "debug",		0, 0, -300  },
    { "quiet",		0, 0, 'q'   },
    { "silent",		0, 0, 'q'   },
    { "verbose",	0, 0, -301  },
    { 0,		0, 0, 0	    }
};

static const char msg_opt_list[] = "q";

struct component comp_msg = {
    "messaging",  msg_init, msg_fini,
    { { msg_opts, msg_opt_list, msg_parse_args } }
};
