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

#pragma ident "@(#)util.c	1.2	99/10/28 meem"

#include <sys/types.h>
#include <config.h>
#include <stddef.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>		/* select() */
#include <string.h>		/* memset() */
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdlib.h>

#ifdef	HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include "util.h"
#include "msg.h"

const char *
h_strerror(void)
{
#ifndef	H_ERRNO_DECLARED
    extern int		h_errno;
#endif
    unsigned int	i;

    static struct {

	int		error;
	const char     *error_string;

    } errlist[] = {

	{ HOST_NOT_FOUND, "specified host is unknown"			 },
	{ NO_ADDRESS,	  "the request is valid but has no IP address"	 },
	{ NO_RECOVERY,	  "a non-recoverable name server error occurred" },
	{ TRY_AGAIN,	  "a temporary error occurred. try again"	 }
    };

    for (i = 0; i < (sizeof errlist / sizeof *errlist); i++)
	if (h_errno == errlist[i].error)
	    return errlist[i].error_string;

    return "unknown error";
}

int
init_sockaddr_in(struct sockaddr_in *sin, const char *host, u_short port_hbo)
{
    struct hostent	*hp;

    memset(sin, 0, sizeof (struct sockaddr_in));
    sin->sin_family	= AF_INET;
    sin->sin_port	= port_hbo ? htons(port_hbo) : 0;

    if (host != 0) {

	if ((hp = gethostbyname(host)) == 0) {
	    msg(R_ERROR, 0, "gethostbyname(%s): %s", host, h_strerror());
	    return 0;
	}

	memcpy(&sin->sin_addr, hp->h_addr, hp->h_length);
    }

    return 1;
}

int
bind_try_range(struct sockaddr_in *sin, int fd, u_short low, u_short high)
{
    while (low <= high) {
	sin->sin_port = htons(low++);
	if (bind(fd, (struct sockaddr *)sin, sizeof (*sin)) == 0)
	    return 1;
    }
    return 0;
}

void
toggle_root(void)
{
    static uid_t saved_setuid;
    static int	 is_initialized = 0;
    uid_t	 new_euid;
    int		 result;

    if (is_initialized == 0) {
	saved_setuid = geteuid();
	is_initialized++;
    }

    new_euid = geteuid() == saved_setuid ? getuid() : saved_setuid;

#ifdef	HAVE_SETEUID
    result = seteuid(new_euid);
#elif	HAVE_SETREUID
    result = setreuid(-1, new_euid);
#elif	HAVE_SETRESUID
    result = setresuid(-1, new_euid, -1);
#else
#error	"This unix variant cannot provide secure operation for the rlpr package"
#endif

    /* usually, i wouldn't call R_FATAL from a utility function, but... */
    if (result == -1)
	msg(R_FATAL, errno, "change of effective uid failed");
}

/*
 * attempt to perform i/o on buffer `buffer' of `buffer_size' to/from
 * file descriptor `fd'.  if `timeout' amount of time transpires
 * before the full i/o operation completes, call `timeout_handler',
 * passing it the number of timeouts so far on this operation, the
 * number of bytes processed so far, and the total number of bytes to
 * process.  if the timeout handler returns nonzero, reset the timeout
 * interval and try again.  in all other cases, including unexpected
 * errors from system calls, we return the number of bytes processed
 * so far.  a return value of less than `buffer_size' is an indication
 * of more serious problems.  callers should probably call R_FATAL or
 * at the very least R_WARNING and try again.
 */

static ssize_t
full_io_internal(int fd, caddr_t buffer, size_t buffer_size, int timeout,
		 rlpr_io_type_e io_type, timeout_handler_t timeout_handler,
		 rlpr_io_status_e *status)
{
    /*
     * we might think to use SO_{SND,RCV}LOWAT to simplify the code
     * here.  there are two reasons we don't use them:
     *
     *	  o SO_{SND,RCV}LOWAT are not specified in POSIX.1g.
     *
     *	  o if `buffer_size' > SO_{SND,RCV}BUF, we're going to have to
     *	    handle `buffer' in fragments anyway, so there's no win.
     */

    struct timeval	start_time, end_time, time_spent;
    struct timeval	time_left;
    struct timeval     *select_time = timeout == -1 ? 0 : &time_left;
    fd_set	       *fds_arr[] = { 0, 0 };
    fd_set		fds;
    ssize_t		n_bytes = 0;	/* total bytes processed */
    unsigned int	n_tries = 1;	/* total number of timeouts */
    ssize_t		pr_bytes;	/* per-round bytes processed */

    time_left.tv_sec = timeout;
    time_left.tv_usec = 0;

    fds_arr[io_type == R_WRITE] = &fds;
    *status = 0;

    while (n_bytes < buffer_size) {

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	if (gettimeofday(&start_time, 0) == -1) {
	    *status = R_IO_ERROR;
	    return n_bytes;
	}

	switch (select(fd + 1, fds_arr[0], fds_arr[1], 0, select_time)) {

	case  0:

	    if (timeout_handler == 0 ||
		timeout_handler(n_tries++, n_bytes, buffer_size) == 0) {
		*status = R_IO_TIMEOUT;
		return n_bytes;
	    }

	    time_left.tv_sec = timeout;
	    continue;

	case -1:

	    *status = R_IO_ERROR;
	    return n_bytes;
	}

	/* we only get here if a file descriptor is ready */

	if (gettimeofday(&end_time, 0) == -1) {
	    *status = R_IO_ERROR;
	    return n_bytes;
	}

	if (io_type == R_READ)
	    pr_bytes =	read(fd, &buffer[n_bytes], buffer_size - n_bytes);
	else
	    pr_bytes = write(fd, &buffer[n_bytes], buffer_size - n_bytes);

	time_spent.tv_sec  = end_time.tv_sec  - start_time.tv_sec;
	time_spent.tv_usec = end_time.tv_usec - start_time.tv_usec;
	if (time_spent.tv_usec < 0) {
	    time_spent.tv_sec--;
	    time_spent.tv_usec += 1000000; /* one second */
	}

	time_left.tv_sec  -= time_spent.tv_sec;
	time_left.tv_usec -= time_spent.tv_usec;
	if (time_left.tv_usec < 0) {
	    time_left.tv_sec--;
	    time_left.tv_usec += 1000000;  /* one second */
	}

	/*
	 * since our computation of time elapsed is not atomic with
	 * respect to the calls to select(), it's possible that we
	 * think we have negative time left.  in this case, reset
	 * time left to be zero and let select() handle it.
	 */

	if (time_left.tv_sec < 0) {
	    time_left.tv_sec  = 0;
	    time_left.tv_usec = 0;
	}

	if (pr_bytes == 0) {
	    *status = R_IO_EOF;
	    return n_bytes;
	}

	if (pr_bytes == -1) {
	    if (errno != EINTR) {
		continue;
	    } else {
		*status = R_IO_ERROR;
		return n_bytes;
	    }
	}

	n_bytes += pr_bytes;
    }

    return n_bytes;
}

ssize_t
full_read_timed(int fd, void *buffer, size_t buffer_size, int timeout_secs,
		timeout_handler_t timeout_handler, rlpr_io_status_e *statusp)
{
    rlpr_io_status_e	status;
    ssize_t		n_read;

    n_read = full_io_internal(fd, (caddr_t)buffer, buffer_size, timeout_secs,
			      R_READ, timeout_handler, &status);
    switch (status) {

    case R_IO_ERROR:

	if (statusp == 0 || (*statusp & R_IO_ERROR) == 0)
	    msg(R_FATAL, errno, "full_read_timed: unexpected error");
	break;

    case R_IO_TIMEOUT:

	if (statusp == 0 || (*statusp & R_IO_TIMEOUT) == 0)
	    msg(R_FATAL, 0, "full_read_timed: timed out after %d seconds",
		timeout_secs);
	break;

    case R_IO_EOF:

	if (statusp == 0 || (*statusp & R_IO_EOF) == 0)
	    msg(R_FATAL, 0, "full_read_timed: unexpected EOF");
	break;

    default:

	break;
    }

    if (statusp != 0)
	*statusp = status;

    return n_read;
}

ssize_t
full_write_timed(int fd, const void *buffer, size_t buffer_size,
		 int timeout_secs, timeout_handler_t timeout_handler,
		 rlpr_io_status_e *statusp)
{
    rlpr_io_status_e	status;
    ssize_t		n_written;

    n_written = full_io_internal(fd, (caddr_t)buffer, buffer_size, timeout_secs,
				 R_WRITE, timeout_handler, &status);
    switch (status) {

    case R_IO_ERROR:

	if (statusp == 0 || (*statusp & R_IO_ERROR) == 0)
	    msg(R_FATAL, errno, "full_write_timed: unexpected error");
	break;

    case R_IO_TIMEOUT:

	if (statusp == 0 || (*statusp & R_IO_TIMEOUT) == 0)
	    msg(R_FATAL, 0, "full_write_timed: timed out after %d seconds",
		timeout_secs);
	break;

    default:

	break;
    }

    if (statusp != 0)
	*statusp = status;

    return n_written;
}

ssize_t
full_write(int fd, const void *buffer, size_t buffer_size,
	   rlpr_io_status_e *status)
{
    if (full_write_timed(fd, buffer, buffer_size, -1, 0, status) == buffer_size)
	return 1;

    return 0;
}

int
check_ack(int sock_fd, const char *what, int timeout)
{
    char		ack;
    rlpr_io_status_e	status = R_IO_TIMEOUT;

    if (full_read_timed(sock_fd, &ack, 1, timeout, 0, &status) < 1) {
	msg(R_ERROR, 0, "check_ack: timed out waiting for ACK to our %s", what);
	return 0;
    }

    if (ack != 0) {
	msg(R_ERROR, 0, "check_ack: lpd refused our %s [error %d]"
	    " -- is our hostname in its hosts.lpd?", what, ack);
	return 0;
    }

    msg(R_DEBUG, 0, "check_ack: received valid ACK to %s", what);
    return 1;
}

off_t
file_size(int fd)
{
    struct stat	stat_buf;

    if (fstat(fd, &stat_buf) == -1)
	return (off_t)-1;

    return stat_buf.st_size;
}

int
copy_file(int in_fd, int out_fd, int timeout, off_t in_fsize, const char *what)
{
    ssize_t		n_sent, n_read, total_sent = 0;
    char		buffer[R_BUFMAX];
    rlpr_io_status_e	status;

    for (;;) {

	status = R_IO_EOF;
	n_read = full_read_timed(in_fd, buffer, sizeof buffer, timeout, 0,
				 &status);
	switch (status) {

	case R_IO_EOF:
	    full_write_timed(out_fd, buffer, n_read, timeout, 0, 0);
	    return 1;

	default:
	    break;
	}

	n_sent = full_write_timed(out_fd, buffer, n_read, timeout, 0, 0);

	total_sent += n_sent;
	if (in_fsize != 0)
	    msg(R_TTY, 0, "%s: transferred %d%%\r", what, total_sent * 100 /
		in_fsize);
    }
}

int
connect_timed(int sock_fd, struct sockaddr_in *sin, int timeout_secs)
{
    int			retval;
    int			orig_flags;
    int			error = 0;
    size_t		error_len = sizeof (int);
    fd_set		fds;
    struct timeval	tv;

    orig_flags = fcntl(sock_fd, F_GETFL, 0);
    if (orig_flags == -1)
	return -1;

    fcntl(sock_fd, F_SETFL, O_NONBLOCK|orig_flags);

    retval = connect(sock_fd, (struct sockaddr *)sin, sizeof (*sin));
    if (retval != -1 || errno != EINPROGRESS) {

	/*
	 * system either does not support nonblocking connects, or
	 * the connect() already completed.
	 */

	fcntl(sock_fd, F_SETFL, orig_flags);
	return retval;
    }

    FD_ZERO(&fds);
    FD_SET(sock_fd, &fds);
    tv.tv_sec  = timeout_secs;
    tv.tv_usec = 0;

    switch (select(sock_fd + 1, 0, &fds, 0, &tv)) {

    case -1:
	error = errno;
	break;

    case 0:
	error = ETIMEDOUT;
	break;

    default:

	/*
	 * on solaris (in the past), a pending error has been returned
	 * by having getsockopt() return -1 and setting errno to the pending
	 * error.
	 */

	if (getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &error, &error_len) == -1)
	    error = errno;
	break;
    }

    fcntl(sock_fd, F_SETFL, orig_flags);

    errno = error;
    return error ? -1 : 0;
}

const char **
string_to_args(char *string, int *argc)
{
    unsigned int	i, length, arg = 0;
    const char	      **argv;

    /*
     * first, calculate the number of arguments we're gonna need,
     * then build and set the argv array of pointers.
     */

    *argc = 1;
    for (i = 0; i == 0 || string[i - 1] != '\0'; i++) {

	while (isspace(string[i]))
	    i++;

	if (string[i] != '\0') {
	    (*argc)++;
	    while (isgraph(string[i]))
		i++;
	}
    }

    length = i;
    argv   = malloc((sizeof (char **)) * (*argc));

    if (argv != 0) {
	argv[arg++] = program_name;

	for (i = 0; i < length; i++) {

	    while (isspace(string[i]))
		i++;

	    if (i < length) {
		argv[arg++] = &string[i];
		while (isgraph(string[i]))
		    i++;

		string[i] = '\0';
	    }
	}
    }

    return argv;
}
