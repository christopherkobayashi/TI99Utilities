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

#pragma ident "@(#)client.c	1.2	99/10/29 meem"

#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <lib.h>		/* xmalloc, etc */
#include <stdio.h>		/* EOF */
#include <getopt.h>
#include <xstrtol.h>

#include "util.h"
#include "rlprrc.h"
#include "msg.h"
#include "client.h"
#include "rlprd.h"		/* R_RLRPRD_LISTEN_PORT */

static struct rlpr_client	*rlpr_client;

static int
client_init(void)
{
    struct utsname	utsname;

    if (uname(&utsname) == -1) {
	msg(R_ERROR, errno, "uname failed");
	return 0;
    }

    rlpr_client			= xmalloc(sizeof (struct rlpr_client));
    rlpr_client->src_port	= 0;
    rlpr_client->dst_port	= 0;
    rlpr_client->no_bind	= 0;
    rlpr_client->localhost	= xstrdup(utsname.nodename);
    rlpr_client->proxyhost	= getenv("RLPR_PROXYHOST");
    rlpr_client->printhost	= getenv("RLPR_PRINTHOST");
    rlpr_client->printer	= getenv("PRINTER");
    if (rlpr_client->printer == 0)
	rlpr_client->printer	= getenv("LPDEST");

    return 1;
}

static int
client_finish_init(void)
{
    char		*at_sign;

    if (rlpr_client->printer)
	if ((at_sign = strchr(rlpr_client->printer, '@')) != 0) {
	    *at_sign++ = '\0';
	    rlpr_client->printhost = at_sign;
	}

    /*
     * if the user hasn't hand-picked a port, then let's go find one
     * ourselves.  if there's a proxy involved, assume it's necessary
     * and try connecting to the proxy port.  otherwise, try to look
     * up the `printer' service in the services database, and if that
     * fails, just fallback on a hardcoded number.
     */

    if (rlpr_client->dst_port == 0) {
	if (rlpr_client->proxyhost != 0) {
	    rlpr_client->dst_port = R_RLPRD_LISTEN_PORT;
	} else {
	    struct servent *sp = getservbyname("printer", "tcp");
	    rlpr_client->dst_port = sp ? ntohs(sp->s_port) : R_LPD_DST_PORT;
	}
    }

    if (rlpr_client->printhost == 0 && rlpr_client->printer == 0) {
	msg(R_ERROR, 0, "neither printhost nor printer set");
	return 0;
    }

    if (rlpr_client->printhost == 0) {
	rlpr_client->printhost = rlprrc(rlpr_client->printer, R_RESOLVE_HOST);
	if (rlpr_client->printhost == 0) {

	    /*
	     * cut the user slack here: it's likely they want to print
	     * locally.  inform them of our intentions and continue..
	     */

	    msg(R_INFO, 0, "cannot resolve printhost for printer `%s', "
		"assuming `%s'.", rlpr_client->printer,
		rlpr_client->localhost);

	    rlpr_client->printhost = rlpr_client->localhost;
	    return 1;
	}
    }

    if (rlpr_client->printer == 0) {
	rlpr_client->printer = rlprrc(rlpr_client->printhost, R_RESOLVE_QUEUE);
	if (rlpr_client->printer == 0) {

	    /*
	     * presever weird bsd "default to lp" behavior.
	     */

	    msg(R_INFO, 0, "cannot resolve printer for printhost `%s', "
		"assuming `lp'.", rlpr_client->printhost);
	    rlpr_client->printer = "lp";
	}
    }

    return 1;
}

static int
client_parse_args(int opt)
{
    switch (opt) {

    case -200:
	rlpr_client->dst_port = strtoul(optarg, 0, 0);
	break;

    case 'H':
	rlpr_client->printhost = optarg;
	break;

    case 'N':
	rlpr_client->no_bind++;
	break;

    case 'P':
    case 'Q':
	rlpr_client->printer = optarg;
	break;

    case 'X':
	rlpr_client->proxyhost = optarg;
	break;

    case EOF:
	return client_finish_init();
    }

    return 1;
}

int
client_open(int timeout)
{
    int			sin_size = sizeof (struct sockaddr_in);
    struct sockaddr_in	sin_local;
    struct sockaddr_in	sin_dst;
    int			sock_fd;
    const char	       *dst_host;

    init_sockaddr_in(&sin_local, 0, 0);

    /*
     * create our local socket descriptor.  pay attention:
     *
     * SVR4 STREAMS-based socket implementations check permissions
     * on binds to privileged ports relative to the credentials of
     * the socket creator.  thus, if we want to bind to a privileged
     * port, we should become root before creating the socket.
     */

    if (rlpr_client->proxyhost == 0 && rlpr_client->no_bind == 0) {

	/*
	 * become root, but even if we can't become root (perhaps
	 * because we're not setuid), keep on going anyway, since
	 * some lpd implementations will allow connections from
	 * non-privileged ports.
	 */

	toggle_root();
	if (geteuid() != 0)
	    msg(R_WARNING, 0, "cannot bind to privileged port: lpd may reject");
    }

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
	if (geteuid() == 0)
	    toggle_root();
	msg(R_ERROR, errno, "socket create for connection to lpd");
	return -1;
    }

    if (geteuid() == 0 && rlpr_client->no_bind == 0) {

	if (bind_try_range(&sin_local, sock_fd, R_LPD_SRC_PORT_LOW,
			   R_LPD_SRC_PORT_HIGH) == 0) {
	    toggle_root();
	    msg(R_FATAL, errno, "privileged bind failed (no ports left?)");
	    return -1;
	}
	toggle_root();
    }

    /*
     * connect to the remote endpoint.  for debugging purposes,
     * grab our local identity too.
     */

    dst_host = rlpr_client->proxyhost ? rlpr_client->proxyhost :
	       rlpr_client->printhost;
    if (init_sockaddr_in(&sin_dst, dst_host, rlpr_client->dst_port) == 0)
	return -1;

    if (connect_timed(sock_fd, &sin_dst, timeout) == -1) {
	msg(R_ERROR, errno, "connect to %s:%hi", dst_host,
	    rlpr_client->dst_port);
	return -1;
    }

    if (getsockname(sock_fd, (struct sockaddr *)&sin_local, &sin_size) == -1) {
	msg(R_ERROR, errno, "getsockname on connection to lpd");
	return -1;
    }

    rlpr_client->src_port = ntohs(sin_local.sin_port);
    msg(R_DEBUG, 0, "connected localhost:%hu -> %s:%hu", rlpr_client->src_port,
	dst_host, rlpr_client->dst_port);

    if (rlpr_client->proxyhost != 0) {

	size_t	printhost_len = strlen(rlpr_client->printhost);

	msg(R_DEBUG, 0, "pointing proxy to %s...", rlpr_client->printhost);

	if (full_write(sock_fd, rlpr_client->printhost, printhost_len, 0)
	    == 0 || full_write(sock_fd, "\n", 1, 0) == 0) {
	    msg(R_ERROR, 0, "unable to send printhost name to proxyhost");
	    return -1;
	}
    }

    return sock_fd;
}

int
client_command(int sock_fd, const char *cmd, int timeout, const char *what)
{
    if (client_command_noack(sock_fd, cmd, timeout, what) == 0)
	return 0;

    return check_ack(sock_fd, what, timeout) == 1;
}

int
client_command_noack(int sock_fd, const char *cmd, int timeout,
    const char *what)
{
    size_t	cmdlen = strlen(cmd);

    msg(R_DEBUG, 0, "client_command: sending `%s' operation", what);
    if (full_write_timed(sock_fd, cmd, cmdlen, timeout, 0, 0) < cmdlen) {
	msg(R_ERROR, errno, "full_write_timed for %s", what);
	return 0;
    }

    return 1;
}

const char *
client_get_printer(void)
{
    return rlpr_client->printer ? rlpr_client->printer : "(unknown)";
}

const char *
client_get_proxyhost(void)
{
    return rlpr_client->proxyhost ? rlpr_client->proxyhost : "(none)";
}

const char *
client_get_printhost(void)
{
    return rlpr_client->printhost ? rlpr_client->printhost : "(unknown)";
}

static struct option client_opts[] = {
    { "no-bind",	0, 0, 'N'  },
    { "port",		1, 0, -200 },
    { "printer",	1, 0, 'P'  },
    { "queue",		1, 0, 'P'  },
    { "printhost",	1, 0, 'H'  },
    { "proxyhost",	1, 0, 'X'  },
    { 0,		0, 0,  0   }
};

static const char client_opt_list[] = "H:NP:Q:X:";

struct component comp_client = {
    "client", client_init, 0,
    { { client_opts, client_opt_list, client_parse_args } }
};
