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

#pragma ident "@(#)rlprm.c	1.1	99/09/16 meem"

#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <lib.h>

#include "intl.h"
#include "component.h"
#include "msg.h"
#include "util.h"
#include "client.h"
#include "rlprrc.h"
#include "rlprm.h"
#include "rfc1179.h"

static struct component *components[] =
{
    &comp_msg,
    &comp_component,
    &comp_client,
    &comp_rlprm,
    0
};

const char		 *program_name;
const char		 *bsd_program_name = "lprm";

static struct rlpr_rlprm *rlpr_rlprm;

static const char	 *make_removej_request(const char *, char * const *);

int
main(int argc, char *argv[])
{
    const char	       *req;
    struct component   *comp;
    int			sock_fd;
    int			timeout;

    program_name = argv[0];
    toggle_root();

    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    if ((comp = component_init(components, argc, argv)) != 0)
	msg(R_FATAL, 0, "component `%s': init() failed!", comp->name);

    if (argv[optind] && strcmp("-", argv[optind]) == 0) {
	argv[optind] = rlpr_rlprm->user;
	if (getuid() == 0)
	    msg(R_WARNING, 0, "`-' option as root only removes root's"
		" jobs (see BUGS)");
    }

    /* just to tidy the code later */
    timeout = rlpr_rlprm->timeout;

    /*
     * make the remove job request message (which we need to make as
     * one message because some lpd's are broken and require that they
     * reap the message in one read(2)).  then connect to the lpd,
     * send the request and read the reply.
     */

    req = make_removej_request(client_get_printer(), &argv[optind]);

    sock_fd = client_open(rlpr_rlprm->timeout);
    if (sock_fd == -1)
	msg(R_FATAL, 0, "client_open(): cannot connect to lpd");

    if (client_command_noack(sock_fd, req, timeout, "remove job req") == 0)
	msg(R_FATAL, 0, "unable to send `remove job' request to lpd");

    if (!msg_is_quiet())
	if (copy_file(sock_fd, STDOUT_FILENO, timeout, 0, "stdout") == 0)
	    msg(R_FATAL, 0, "unable to get job removal output from lpd");

    close(sock_fd);

    if ((comp = component_fini(components)) != 0)
	msg(R_FATAL, 0, "component `%s': fini() failed!", comp->name);

    return EXIT_SUCCESS;
}

static const char *
make_removej_request(const char *printer, char * const *argv)
{
    unsigned int	i;
    unsigned int	request_size = 3; /* REMJ"\n" */
    char	       *request;

    request_size += strlen(rlpr_rlprm->user) + 1;
    request_size += strlen(printer);
    for (i = 0; argv[i] != 0; i++)
	request_size += strlen(argv[i]) + 1;

    request = xmalloc(request_size);
    sprintf(request, "%c%s %s", REMJ, printer, rlpr_rlprm->user);

    for (i = 0; argv[i] != 0; i++) {
	strcat(request, " ");
	strcat(request, argv[i]);
    }

    strcat(request, "\n");
    return request;
}

static int
rlprm_init(void)
{
    struct passwd      *pwd;

    rlpr_rlprm	= xmalloc(sizeof (struct rlpr_rlprm));
    pwd = getpwuid(getuid());

    if (pwd == 0 || pwd->pw_name == 0) {
	msg(R_ERROR, errno, "unable to resolve your username");
	return 0;
    }

    rlpr_rlprm->timeout = R_TIMEOUT_DEFAULT;
    rlpr_rlprm->user	= xstrdup(pwd->pw_name);
    return 1;
}

static int
rlprm_parse_args(int opt)
{
    switch (opt) {

    case -600:
	msg(R_STDOUT, 0, "usage: %s [-Hprinthost] [-Pprinter] [-Xproxy]"
	    " [OPTIONS] [job/user ...]\nplease see the manpage for"
	    " detailed help", program_name);
	exit(EXIT_SUCCESS);
	break;

    case -601:
	rlpr_rlprm->timeout = strtol(optarg, 0, 0);
	break;

    case 'V':
	msg(R_STDOUT, 0, "version "VERSION" from "__DATE__" "__TIME__
	    " -- meem@gnu.org");
	exit(EXIT_SUCCESS);
	break;

    case EOF:
	break;
    }

    return 1;
}

static struct option rlprm_opts[] = {
    { "help",		0, 0, -600  },
    { "timeout",	1, 0, -601  },
    { "version",	0, 0, 'V'   },
    { 0,		0, 0,  0    }
};

static const char rlprm_opt_list[] = "V";

struct component comp_rlprm = {
    "rlprm", rlprm_init, 0,
    { { rlprm_opts, rlprm_opt_list, rlprm_parse_args } }
};
