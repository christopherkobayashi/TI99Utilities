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

#pragma ident "@(#)client.h	1.1	99/09/16 meem"

#ifndef	CLIENT_H
#define	CLIENT_H

#include "component.h"
#include "rfc1179.h"

/*
 * bsd compatibility
 */

#define	R_PRINTER_DEFAULT	"lp"

/*
 * TODO: ideally the ushort_t's would be in_port_t's or uint16_t's,
 * but unfortunately, neither type is widely available yet.
 */

struct rlpr_client
{
    const char	       *localhost;
    const char	       *proxyhost;
    const char	       *printhost;
    const char	       *printer;

    u_short		dst_port;
    u_short		src_port;

    unsigned char	no_bind;
};

/* component-specific operations  */

int		client_open(int);
int		client_command(int, const char *, int, const char *);
int		client_command_noack(int, const char *, int, const char *);
const char *	client_get_printer(void);
const char *	client_get_printhost(void);
const char *	client_get_proxyhost(void);

extern struct component comp_client;

#endif	/* CLIENT_H */
