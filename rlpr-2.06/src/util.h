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

#pragma ident "@(#)util.h	1.1	99/09/16 meem"

#ifndef	UTIL_H
#define	UTIL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef	MAX
#define	MAX(a, b)	((a) > (b) ? (a) : (b))
#endif

#ifndef	MIN
#define	MIN(a, b)	((a) < (b) ? (a) : (b))
#endif

#define	R_BUFMAX		1024		/* bytes */
#define	R_TIMEOUT_DEFAULT	25		/* seconds */

typedef	enum { R_READ, R_WRITE } rlpr_io_type_e;
typedef enum { R_IO_EOF = 1, R_IO_TIMEOUT, R_IO_ERROR } rlpr_io_status_e;

typedef int   (*timeout_handler_t)(unsigned int, ssize_t, size_t);

/* utility functions */

const char **	string_to_args(char *, int *);

int		bind_try_range(struct sockaddr_in *, int, u_short, u_short);

const char *	h_strerror(void);

int		init_sockaddr_in(struct sockaddr_in *, const char *, u_short);

void		toggle_root(void);

int		check_ack(int, const char *, int);

off_t		file_size(int);

int		copy_file(int, int, int, off_t, const char *);

int		connect_timed(int, struct sockaddr_in *, int);

/* full-i/o operations */

ssize_t		full_write(int, const void *, size_t, rlpr_io_status_e *);

ssize_t		full_read_timed(int, void *, size_t, int, timeout_handler_t,
				rlpr_io_status_e *);

ssize_t		full_write_timed(int, const void *, size_t, int,
				 timeout_handler_t, rlpr_io_status_e *);
#endif /* UTIL_H */
