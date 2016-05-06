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

#pragma ident "@(#)rlprd.c	1.1	99/09/16 meem"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>		/* select() */
#include <sys/stat.h>		/* umask() */
#include <unistd.h>
#include <lib.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <config.h>
#include R_MAXHOSTNAMELEN_HDR

#ifdef	 HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include "intl.h"
#include "util.h"
#include "msg.h"
#include "rlprd.h"
#include "rfc1179.h"

static struct component *components[] =
{
    &comp_msg,
    &comp_component,
    &comp_rlprd,
    0
};

const char		 *program_name;
const char		 *bsd_program_name = 0;

static struct rlpr_rlprd *rlpr_rlprd;

static int		converse(int, int);
static int		process_client(int, struct sockaddr_in *);
static int		register_sigchld(void);
static int		daemonize(void);
static RETSIGTYPE	catch_sigchld(int);
static RETSIGTYPE	graceful_shutdown(int);

int
main(int argc, char *argv[])
{
    struct component   *comp;
    int			listen_fd, client_fd;
    struct sockaddr_in	sin_rlprd, sin_client;
    int			unused, on = 1;

    program_name = argv[0];
    toggle_root();

    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    if ((comp = component_init(components, argc, argv)) != 0)
	msg(R_FATAL, 0, "component `%s': init() failed!", comp->name);

    if (geteuid() != 0)
	msg(R_FATAL, 0, "must be run as root or setuid root");

    if (register_sigchld() == -1)
	msg(R_FATAL, errno, "register_sigchld");

    signal(SIGHUP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, graceful_shutdown);

    if (rlpr_rlprd->no_daemon == 0 && daemonize() == 0)
	msg(R_FATAL, 0, "daemonizing failed");

    /*
     * open ourselves for business
     */

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
	msg(R_FATAL, errno, "socket() for listening socket");

    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (caddr_t)&on, sizeof on);

    init_sockaddr_in(&sin_rlprd, 0, rlpr_rlprd->listen_port);

    if (bind(listen_fd, (struct sockaddr *)&sin_rlprd, sizeof sin_rlprd) == -1)
	msg(R_FATAL, errno, "bind() on listening socket");

    if (listen(listen_fd, 5) == -1)
	msg(R_FATAL, errno, "listen() on listening socket");

    msg(R_DEBUG, 0, "listening for incoming connections on port %hu",
	rlpr_rlprd->listen_port);

    /*
     * main loop - accept incoming requests, calling process_client() on each
     */

    for (;;) {

	client_fd = accept(listen_fd, (struct sockaddr *)&sin_client, &unused);
	if (client_fd == -1) {
	    if (errno != EINTR)
		msg(R_ERROR, errno, "accept");
	    continue;
	}

	switch (fork()) {

	case  0:
	    if (process_client(client_fd, &sin_client) == 0)
		exit(EXIT_FAILURE);

	    exit(EXIT_SUCCESS);
	    break;

	case -1:
	    msg(R_ERROR, errno, "fork");
	    break;

	default:
	    close(client_fd);
	    break;
	}
    }

    /* NOTREACHED */
    return EXIT_SUCCESS;
}

static int
process_client(int client_fd, struct sockaddr_in *sin_client)
{
    char		printhost[MAXHOSTNAMELEN + 1];
    int			lpd_fd;
    unsigned int	i;
    struct sockaddr_in	sin_local;
    struct sockaddr_in	sin_lpd;

    msg(R_DEBUG, 0, "process_client: connection from %s:%i",
	inet_ntoa(sin_client->sin_addr), ntohs(sin_client->sin_port));

    /*
     * read in the hostname
     */

    for (i = 0; i < MAXHOSTNAMELEN; i++) {
	full_read_timed(client_fd, &printhost[i], 1, rlpr_rlprd->timeout, 0, 0);
	if (printhost[i] == '\n')
	    break;
    }
    printhost[i] = '\0';
    msg(R_DEBUG, 0, "process_client: request for host `%s'", printhost);

    /*
     * process the request
     */

    init_sockaddr_in(&sin_local, 0, 0);
    if (init_sockaddr_in(&sin_lpd, printhost, R_LPD_DST_PORT) == 0)
	return 0;

    toggle_root();			/* get root */

    lpd_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (lpd_fd == -1) {
	msg(R_ERROR, errno, "process_client: socket");
	toggle_root();
	return 0;
    }

    if (bind_try_range(&sin_local, lpd_fd, R_LPD_SRC_PORT_LOW,
		       R_LPD_SRC_PORT_HIGH) == 0) {
	msg(R_ERROR, errno, "process_client: bind");
	toggle_root();
	return 0;
    }

    toggle_root();

    if (connect_timed(lpd_fd, &sin_lpd, rlpr_rlprd->timeout) == -1) {
	msg(R_ERROR, errno, "process_client: connect to lpd");
	return 0;
    }

    msg(R_DEBUG, 0, "process_client: connected from %s:%i to %s:%i",
	inet_ntoa(sin_client->sin_addr), ntohs(sin_client->sin_port),
	inet_ntoa(sin_lpd.sin_addr), ntohs(sin_lpd.sin_port));

    if (converse(client_fd, lpd_fd) == 0) {
	msg(R_ERROR, 0, "process_client: errors in conversing with lpd");
	return 0;
    }

    msg(R_DEBUG, 0, "process client: transaction completed");
    return 1;
}

static int
converse(int client_fd, int server_fd)
{
    int			maxfdp1 = MAX(client_fd, server_fd) + 1;
    char		buffer[R_BUFMAX];
    fd_set		fds;
    int			read_fd, write_fd;
    ssize_t		n_read;

    for (;;) {

	FD_ZERO(&fds);
	FD_SET(client_fd, &fds);
	FD_SET(server_fd, &fds);

	if (select(maxfdp1, &fds, 0, 0, 0) <= 0) {
	    msg(R_ERROR, errno, "converse: select");
	    return 0;
	}

	read_fd	 = FD_ISSET(client_fd, &fds) ? client_fd : server_fd;
	write_fd = FD_ISSET(client_fd, &fds) ? server_fd : client_fd;

	n_read = read(read_fd, buffer, sizeof buffer);
	if (n_read == 0)
	    return 1;

	full_write_timed(write_fd, buffer, n_read, rlpr_rlprd->timeout, 0, 0);
    }
}

/* ARGSUSED */
static RETSIGTYPE
catch_sigchld(int unused)
{
    while (waitpid(-1, 0, WNOHANG) > 0)
	;
}

/* ARGSUSED */
static RETSIGTYPE
graceful_shutdown(int unused)
{
    struct component	*comp;

    if ((comp = component_fini(components)) != 0)
	msg(R_FATAL, 0, "component `%s': fini() failed!", comp->name);

    exit(EXIT_SUCCESS);
}

static int
register_sigchld(void)
{
    struct sigaction	sa = { 0 };

    sa.sa_handler = catch_sigchld;
#ifdef	SA_RESTART
    sa.sa_flags	  = SA_RESTART;
#endif
    sigemptyset(&sa.sa_mask);

    return sigaction(SIGCHLD, &sa, 0);
}

static int
daemonize(void)
{
    msg_use_syslog(1);

    switch (fork()) {

    case -1:

	msg(R_ERROR, errno, "fork while daemonizing");
	break;

    case  0:

	/*
	 * start our new session, fork() again to lose process group leader
	 */

	setsid();
	switch (fork()) {

	case 0:

	    chdir("/");
	    umask(0);
	    return 1;

	default:
	    break;
	}

	/* FALLTHRU into default */

    default:
	exit(EXIT_SUCCESS);
    }

    return 0;
}

static int
rlprd_init(void)
{
    struct rlpr_rlprd	rlpr_rlprd_tmpl = { 0, R_RLPRD_LISTEN_PORT };

    rlpr_rlprd	= xmalloc(sizeof (struct rlpr_rlprd));
    *rlpr_rlprd = rlpr_rlprd_tmpl;

    rlpr_rlprd->timeout = R_TIMEOUT_DEFAULT;
    return 1;
}

static int
rlprd_parse_args(int opt)
{
    switch (opt) {

    case 'n':
	rlpr_rlprd->no_daemon++;
	break;

    case 'p':
	rlpr_rlprd->listen_port = strtoul(optarg, 0, 0);
	break;

    case -700:
	msg(R_STDOUT, 0, "usage: %s [-nqV] [-pport] [--debug]"
	    " \nplease see the manpage for detailed help", program_name);
	exit(EXIT_SUCCESS);
	break;

    case -701:
	rlpr_rlprd->timeout = strtol(optarg, 0, 0);
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

static struct option rlprd_opts[] = {
    { "help",		0, 0, -700  },
    { "no-daemon",	0, 0, 'n'   },
    { "port",		1, 0, 'p'   },
    { "timeout",	1, 0, -701  },
    { "version",	0, 0, 'V'   },
    { 0,		0, 0,  0    }
};

static const char rlprd_opt_list[] = "npV";

struct component comp_rlprd = {
    "rlprd", rlprd_init, 0,
    { { rlprd_opts, rlprd_opt_list, rlprd_parse_args } }
};
