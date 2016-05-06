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

#pragma ident "@(#)rlpq.c	1.1	99/09/16 meem"

#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <lib.h>

#include "intl.h"
#include "component.h"
#include "msg.h"
#include "util.h"
#include "client.h"
#include "rlprrc.h"
#include "rlpq.h"
#include "rfc1179.h"

static struct component *components[] =
{
    &comp_msg,
    &comp_component,
    &comp_client,
    &comp_rlpq,
    0
};

const char		*program_name;
const char		*bsd_program_name = "lpq";

static struct rlpr_rlpq	*rlpr_rlpq;

static const char	*make_queue_request(const char *, char * const *);

int
main(int argc, char *argv[])
{
    const char	       *req;
    struct component   *comp;
    char	       *user_argv[] = { 0, 0 };
    char * const       *user_list = user_argv;
    int			sock_fd;
    int			retval = EXIT_SUCCESS;

    program_name = argv[0];
    toggle_root();

    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    if ((comp = component_init(components, argc, argv)) != 0)
	msg(R_FATAL, 0, "component `%s': init() failed!", comp->name);

    /*
     * if --mine is specified, then use a two-element argv[] that
     * has the first element set to the current user
     */

    if (rlpr_rlpq->mine_only)
	user_argv[0] = rlpr_rlpq->user;
    else
	user_list    = &argv[optind];

    /*
     * make the queue request message (which we need to make as
     * one message because some lpd's are broken and require that
     * they reap the message in one read(2)).  then connect to
     * the lpd, send the request and read the reply.
     */

    req = make_queue_request(client_get_printer(), user_list);

    sock_fd = client_open(rlpr_rlpq->timeout);
    if (sock_fd == -1)
	msg(R_FATAL, 0, "client_open(): cannot connect to lpd");

    if (client_command_noack(sock_fd, req, rlpr_rlpq->timeout, "queue request")
	== 0)
	msg(R_FATAL, 0, "unable to send queue request to lpd");

    if (msg_is_quiet()) {

	char		buffer[BUFSIZ];
	int		pipe_fd[2];
	ssize_t		n_read;

	/*
	 * we'd like to see if there's a "no entries\n" string
	 * returned or not from the lpd.  however, we want to respect
	 * timeouts retrieving this information -- ideally, we'd use
	 * copy_file() since it knows about timeouts.  what to do?
	 * pipes to the rescue.	 we're so friggin' clever sometimes.
	 */

	if (pipe(pipe_fd) == -1)
	    msg(R_FATAL, errno, "pipe");

	if (copy_file(sock_fd, pipe_fd[1], rlpr_rlpq->timeout, 0, "pipe") == 0)
	    msg(R_FATAL, 0, "unable to get queue output from lpd");

	if ((n_read = read(pipe_fd[0], buffer, sizeof buffer - 1)) == -1)
	    msg(R_FATAL, errno, "read of piped data from lpd");

	buffer[n_read] = '\0';
	if (strcasecmp("no entries\n", buffer) == 0)
	    retval = EXIT_FAILURE;

    } else {

	if (copy_file(sock_fd, STDOUT_FILENO, rlpr_rlpq->timeout, 0, "stdout")
	    == 0)
	    msg(R_FATAL, 0, "unable to get queue output from lpd");
    }

    close(sock_fd);

    if ((comp = component_fini(components)) != 0)
	msg(R_FATAL, 0, "component `%s': fini() failed!", comp->name);

    return retval;
}

static const char *
make_queue_request(const char *printer, char * const *user_list)
{
    unsigned int	request_size = 3; /* "\003\n" */
    char	       *request;
    unsigned int	i;

    request_size += strlen(printer);
    for (i = 0; user_list[i] != 0; i++)
	request_size += strlen(user_list[i]) + 1;

    request = xmalloc(request_size);
    sprintf(request, "%c%s", rlpr_rlpq->long_output ? SENDQL : SENDQS, printer);

    for (i = 0; user_list[i] != 0; i++) {
	strcat(request, " ");
	strcat(request, user_list[i]);
    }

    strcat(request, "\n");
    return request;
}

static int
rlpq_init(void)
{
    struct rlpr_rlpq	rlpr_rlpq_tmpl = { 0 };
    struct passwd      *pwd;

    /*
     * we perform a structure copy on a zero-initialized structure
     * instead of using memset() since memset is a raw bit-oriented
     * function, which means that it will generate potentially
     * incorrect results with non-integer types.
     */

    rlpr_rlpq	= xmalloc(sizeof (struct rlpr_rlpq));
    *rlpr_rlpq	= rlpr_rlpq_tmpl;
    pwd = getpwuid(getuid());

    if (pwd == 0 || pwd->pw_name == 0) {
	msg(R_ERROR, errno, "unable to resolve your username");
	return 0;
    }

    rlpr_rlpq->timeout	= R_TIMEOUT_DEFAULT;
    rlpr_rlpq->user	= xstrdup(pwd->pw_name);
    return 1;
}

static int
rlpq_parse_args(int opt)
{
    switch (opt) {

    case 'l':
	rlpr_rlpq->long_output++;
	break;

    case 'm':
	rlpr_rlpq->mine_only++;
	break;

    case -500:
	msg(R_STDOUT, 0, "usage: %s [-Hprinthost] [-Pprinter] [-Xproxy]"
	    " [OPTIONS]\nplease see the manpage for detailed help",
	    program_name);
	exit(EXIT_SUCCESS);
	break;

    case -501:
	rlpr_rlpq->timeout = strtol(optarg, 0, 0);
	break;

    case 'V':
	msg(R_STDOUT, 0, "version "VERSION" from " __DATE__" "__TIME__
	    " -- meem@gnu.org");
	exit(EXIT_SUCCESS);
	break;

    case EOF:
	break;
    }

    return 1;
}

static struct option rlpq_opts[] = {
    { "help",		0, 0, -500  },
    { "long",		0, 0, 'l'   },
    { "mine",		0, 0, 'm'   },
    { "timeout",	1, 0, -501  },
    { "version",	0, 0, 'V'   },
    { 0,		0, 0,  0    }
};

static const char rlpq_opt_list[] = "lmV";

struct component comp_rlpq = {
    "rlpq", rlpq_init, 0,
    { { rlpq_opts, rlpq_opt_list, rlpq_parse_args } }
};
