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

#pragma ident "@(#)rlpr.c	1.2	01/01/01 meem"

#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <lib.h>
#include R_MAXHOSTNAMELEN_HDR

#include "intl.h"
#include "component.h"
#include "msg.h"
#include "util.h"
#include "client.h"
#include "rlprrc.h"
#include "rlpr.h"
#include "rfc1179.h"

static struct component *components[] =
{
    &comp_msg,
    &comp_component,
    &comp_client,
    &comp_rlpr,
    0
};

static struct rlpr_rlpr	*rlpr_rlpr;

static char	*finished_fmt[] = {
    "%i files spooled to %s@%s (proxy %s)",
    "%i file spooled to %s@%s (proxy %s)"
};

const char	*program_name;
const char	*bsd_program_name = "lpr";

static int	send_controlf(int, int, const char *);
static int	send_dataf(int, const char *, const char *);
static void	controlf_add(int, char, const char *, const char *);
static void	controlf_init(int, const char *);
static void	tmp_file_next(int, char *, char *, char *);
static int	tmp_file_init(char **, char **, char **);

int
main(int argc, char *argv[])
{
    struct component	*comp;
    char		*cf_name, *df_name, *cf_path;
    char		*req;
    int			 cf_fd, sock_fd = -1;
    unsigned int	 n_files, files_left, n_copies;

    program_name = argv[0];
    toggle_root();

    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    if ((comp = component_init(components, argc, argv)) != 0)
	msg(R_FATAL, 0, "component `%s': init() failed!", comp->name);

    /*
     * create the name templates for control and data files, and the files..
     */

    cf_fd = tmp_file_init(&cf_path, &cf_name, &df_name);
    if (cf_fd == -1)
	msg(R_FATAL, 0, "cannot create temporary files");

    n_files = argc - optind;
    if (n_files == 0)		/* it's from stdin */
	n_files++;

    /*
     * create the job request.  nasty.
     */

    req = xmalloc(R_REQ_LEN);
    sprintf(req, "%c%.*s\n", RECVJ, R_REQ_LEN - 5, client_get_printer());

    /*
     * now print each file.  note that we have to continuously
     * re-initialize the connection when connecting to windows lpds.
     */

    for (files_left = n_files; files_left != 0; files_left--, optind++) {

	if (sock_fd == -1 || rlpr_rlpr->windows_lpd) {

	    sock_fd = client_open(rlpr_rlpr->timeout);
	    if (sock_fd == -1)
		msg(R_FATAL, 0, "client_open(): cannot connect to lpd");

	    if (client_command(sock_fd, req, rlpr_rlpr->timeout, "job request")
		== 0)
		msg(R_FATAL, 0, "unable to send job request to lpd");
	}

	controlf_init(cf_fd, argv[optind] ? argv[optind] : "stdin");

	for (n_copies = 0; n_copies < rlpr_rlpr->n_copies; n_copies++)
	    controlf_add(cf_fd, rlpr_rlpr->filetype, df_name, 0);

	controlf_add(cf_fd, 'U', df_name, 0);
	controlf_add(cf_fd, 'N', argv[optind], "stdin");

	/*
	 * make sure we can access the file here, before sending the
	 * control file.
	 */

	if (argv[optind] != 0 && access(argv[optind], R_OK) == -1) {

	    msg(R_ERROR, errno, "skipping file `%s'", argv[optind]);
	    n_files--;

	} else {

	    if (rlpr_rlpr->control_first)
		if (send_controlf(sock_fd, cf_fd, cf_name) == 0)
		    msg(R_FATAL, 0, "unable to control file to lpd");

	    if (send_dataf(sock_fd, argv[optind], df_name) == 0)
		msg(R_FATAL, 0, "unable to send file `%s' to lpd",
		    argv[optind] ? argv[optind] : "stdin");

	    if (rlpr_rlpr->control_first == 0)
		if (send_controlf(sock_fd, cf_fd, cf_name) == 0)
		    msg(R_FATAL, 0, "unable to control file to lpd");

	    if (rlpr_rlpr->remove_after && argv[optind])
		if (unlink(argv[optind]) == -1)
		    msg(R_WARNING, errno, "cannot remove `%s'", argv[optind]);
	}

	if (rlpr_rlpr->windows_lpd)
	    close(sock_fd);

	tmp_file_next(cf_fd, cf_path, cf_name, df_name);
    }

    msg(R_INFO, 0, finished_fmt[n_files == 1], n_files,
	client_get_printer(), client_get_printhost(), client_get_proxyhost());

    if ((comp = component_fini(components)) != 0)
	msg(R_FATAL, 0, "component `%s': fini() failed!", comp->name);

    return EXIT_SUCCESS;
}

int
rlpr_init(void)
{
    struct rlpr_rlpr	rlpr_rlpr_tmpl = { 0 };
    struct utsname      utsname;
    struct passwd      *pwd;

    /*
     * yeah, i know this same code is in client_init().  however,
     * there is a method to this madness: their uses of localhost
     * are not tied to each other, and thus neither should their
     * setting of it.
     */

    if (uname(&utsname) == -1) {
	msg(R_ERROR, errno, "uname failed");
	return 0;
    }

    /*
     * we perform a structure copy on a zero-initialized structure
     * instead of using memset() since memset is a raw bit-oriented
     * function, which means that it will generate potentially
     * incorrect results with non-integer types.
     */

    rlpr_rlpr	= xmalloc(sizeof (struct rlpr_rlpr));
    *rlpr_rlpr	= rlpr_rlpr_tmpl;

    /*
     * explicitly initialize all non-zero parameters
     */

    rlpr_rlpr->timeout  = R_TIMEOUT_DEFAULT;
    rlpr_rlpr->n_copies	= 1;
    rlpr_rlpr->filetype = 'f';
    rlpr_rlpr->print_burst++;
    rlpr_rlpr->control_first++;

    pwd = getpwuid(getuid());
    if (pwd == 0 || pwd->pw_name == 0) {
	msg(R_ERROR, errno, "unable to resolve your username");
	return 0;
    }

    rlpr_rlpr->localhost = xstrdup(utsname.nodename);
    rlpr_rlpr->user	 = xstrdup(pwd->pw_name);
    rlpr_rlpr->tmpdir	 = getenv("TMPDIR");
    if (rlpr_rlpr->tmpdir == 0)
	rlpr_rlpr->tmpdir = R_TMPDIR;

    return 1;
}

static int
rlpr_parse_args(int opt)
{
    switch (opt) {

    case '1':		/* troff type R */
    case '2':		/* troff type I */
    case '3':		/* troff type B */
    case '4':		/* troff type S */
    case 'c':		/* cifplot */
    case 'd':		/* TeX/DVI */
    case 'f':		/* fortran */
    case 'g':		/* graph */
    case 'l':		/* leave control characters */
    case 'n':		/* ditroff */
    case 'o':		/* postscript (not in lpr) */
    case 'p':		/* pr */
    case 't':		/* troff */
    case 'v':		/* raster input */

	rlpr_rlpr->filetype = (opt == 'f') ? 'r' : (char)opt;
	break;

    case '#':
	rlpr_rlpr->n_copies = strtoul(optarg, 0, 0);
	break;

    case 'h':
	rlpr_rlpr->print_burst = 0;
	break;

    case 'm':
	rlpr_rlpr->mail_after = 1;
	break;

    case 'r':
	rlpr_rlpr->remove_after = 1;
	break;

    case -405:
	rlpr_rlpr->windows_lpd = 1;
	break;

    case 's':		/* bsd compatibility */
	msg(R_WARNING, 0, "symlink option not applicable (ignored)");
	break;

    case 'C':
	rlpr_rlpr->class = optarg;
	break;

    case 'J':
	rlpr_rlpr->job = optarg;
	break;

    case 'T':
	rlpr_rlpr->title = optarg;
	break;

    case 'U':
	rlpr_rlpr->user = optarg;
	break;

    case -404:
	rlpr_rlpr->tmpdir = optarg;
	break;

    case -401:
	msg(R_STDOUT, 0, "usage: %s [-Hprinthost] [-Pprinter] [-Xproxy]"
	    " [OPTIONS] [file ...]\nplease see the manpage for detailed"
	    " help", program_name);
	exit(EXIT_SUCCESS);
	break;

    case -400:
	rlpr_rlpr->control_first = 0;
	break;

    case -403:
	rlpr_rlpr->timeout = strtol(optarg, 0, 0);
	break;

    case -402:
	rlpr_rlpr->hostname = optarg;
	break;

    case 'V':
	msg(R_STDOUT, 0, "version "VERSION" from " __DATE__" "__TIME__
	    " -- meem@gnu.org");
	exit(EXIT_SUCCESS);
	break;

    case 'w':
	rlpr_rlpr->width = optarg ? optarg : R_NOARG_WIDTH;
	break;

    case 'i':
	rlpr_rlpr->indentation = optarg ? optarg : R_NOARG_INDENT;
	break;

    case EOF:
	break;
    }

    return 1;
}

static int
rlpr_hpux_parse_args(int opt)
{
    msg(R_DEBUG, 0, "optarg: %s", optarg);

    switch (opt) {

    case 'o':
	rlpr_rlpr->options = optarg;
	break;

    case 'p':
	rlpr_rlpr->priority = optarg;
	break;
    }

    /* XXX write_back isn't settable yet; need to check hpux for flag */
    return 1;
}

static off_t
send_header(int sock_fd, int fd, const char *name, char cmd, const char *what)
{
    off_t	fd_size = file_size(fd);
    char	hdr[R_BUFMAX];

    if (fd_size == (off_t)-1) {
	msg(R_ERROR, 0, "send_header: cannot determine size of %s", name);
	return -1;
    }

    sprintf(hdr, "%c"R_OFF_T_FMT" %.*s\n", cmd, fd_size, MAXHOSTNAMELEN, name);
    if (client_command(sock_fd, hdr, rlpr_rlpr->timeout, what) == 0)
	return -1;

    return fd_size;
}

static int
send_controlf(int sock_fd, int cf_fd, const char *cf_name)
{
    char	buffer[R_BUFMAX];
    ssize_t	n_bytes_sent = 0;
    FILE	*cfp;
    off_t	cf_size;

    /*
     * send the control file header
     */

    cf_size = send_header(sock_fd, cf_fd, cf_name, RECVCF, "control file hdr");
    if (cf_size == -1) {
	msg(R_ERROR, 0, "send_controlf: cannot send control file header");
	return 0;
    }

    /*
     * now send the control file over...
     */

    lseek(cf_fd, 0, SEEK_SET);
    cfp = fdopen(cf_fd, "r");
    if (cfp == 0) {
	msg(R_ERROR, errno, "send_controlf: fdopen");
	return 0;
    }

    while (fgets(buffer, sizeof buffer, cfp) != 0) {

	size_t length = strlen(buffer);

	buffer[length - 1] = '\0';
	msg(R_DEBUG, 0, "control file: %s", buffer);
	buffer[length - 1] = '\n';

	if (full_write_timed(sock_fd, buffer, length, rlpr_rlpr->timeout, 0, 0)
	    < length) {
	    msg(R_ERROR, 0, "send_controlf: timed out sending control file");
	    return 0;
	}

	n_bytes_sent += length;
	msg(R_TTY, 0, "control file: sent %d%\r", n_bytes_sent * 100 / cf_size);
    }

    if (full_write_timed(sock_fd, "", 1, rlpr_rlpr->timeout, 0, 0) != 1) {
	msg(R_ERROR, 0, "unable to send control file to lpd");
	return 0;
    }

    return check_ack(sock_fd, "control file contents", rlpr_rlpr->timeout) == 1;
}

static int
send_dataf(int sock_fd, const char *filename, const char *df_name)
{
    int		df_fd;
    off_t	df_size;
    char	*t_filename;

    /*
     * the rfc states tha a data file size of 0 denotes a data file of
     * unspecified size (which is useful when rlpr is streaming from
     * stdin).  however, it appears some lpd's can't cope with this, so we
     * always create a temporary file when reading from stdin.  grrr...
     */

    if (filename != 0) {

	df_fd = open(filename, O_RDONLY);
	if (df_fd == -1) {
	    msg(R_ERROR, errno, "send_dataf: open on `%s'", filename);
	    return 0;
	}

    } else {

	filename   = "stdin";
	t_filename = xmalloc(strlen(rlpr_rlpr->tmpdir) + strlen(df_name) + 2);
	sprintf(t_filename, "%s/%s", rlpr_rlpr->tmpdir, df_name);

	df_fd = open(t_filename, O_RDWR|O_CREAT|O_EXCL, 0600);
	if (df_fd == -1) {
	    msg(R_ERROR, errno, "send_dataf: cannot create temporary datafile");
	    return 0;
	}

	unlink(t_filename);
	free(t_filename);

	if (copy_file(STDIN_FILENO, df_fd, -1, 0, "temporary datafile") == 0) {
	    msg(R_ERROR, 0, "unable to create temporary copy of stdin for lpd");
	    return 0;
	}

	lseek(df_fd, SEEK_SET, 0);
    }

    df_size = send_header(sock_fd, df_fd, df_name, RECVDF, "data file hdr");
    if (df_size == -1) {
	msg(R_ERROR, 0, "send_dataf: sending data file header failed");
	return 0;
    }

    if (df_size == 0)
	msg(R_WARNING, 0, "%s is an empty file", filename);

    if (copy_file(df_fd, sock_fd, rlpr_rlpr->timeout, df_size, filename) == 0) {
	msg(R_ERROR, 0, "unable to send data file to lpd");
	return 0;
    }

    if (full_write_timed(sock_fd, "", 1, rlpr_rlpr->timeout, 0, 0) != 1) {
	msg(R_ERROR, 0, "unable to send data file to lpd");
	return 0;
    }

    return check_ack(sock_fd, "data file contents", rlpr_rlpr->timeout) == 1;
}

static void
controlf_add(int cf_fd, char op, const char *value, const char *def)
{
    char	*entry;

    if (value == 0)
	value = def;

    msg(R_DEBUG, 0, "value %s, default %s", value == 0 ? "(null)" : value,
	def == 0 ? "(null)" : def);

    if (value != 0) {
	entry = xmalloc(strlen(value) + 3);
	sprintf(entry, "%c%s\n", op, value);
	msg(R_DEBUG, 0, "wrote %c%s", op, value);
	if (full_write(cf_fd, entry, strlen(entry), 0) == 0)
	    msg(R_WARNING, errno, "controlf_add: full_write");
	free(entry);
    }
}

static void
controlf_init(int cf_fd, const char *file_to_print)
{
    if (ftruncate(cf_fd, 0) == -1)
	msg(R_WARNING, 0, "controlf_init: ftruncate");

    controlf_add(cf_fd, 'H', rlpr_rlpr->hostname, rlpr_rlpr->localhost);
    controlf_add(cf_fd, 'P', rlpr_rlpr->user, 0);
    controlf_add(cf_fd, 'I', rlpr_rlpr->indentation, 0);
    controlf_add(cf_fd, 'T', rlpr_rlpr->title, 0);

    if (rlpr_rlpr->mail_after)
	controlf_add(cf_fd, 'M', rlpr_rlpr->user, 0);

    if (rlpr_rlpr->print_burst) {

	/*
	 * the defaults on the 'J' and 'C' options are meant to
	 * appease berkeley lpd, which is apparently more finicky than
	 * the rfc calls for.
	 */

	controlf_add(cf_fd, 'J', rlpr_rlpr->job, file_to_print);
	controlf_add(cf_fd, 'C', rlpr_rlpr->class, rlpr_rlpr->localhost);
	controlf_add(cf_fd, 'L', rlpr_rlpr->user, 0);
    }

    /* another berkeleyism.  tolerate. */

    if (strchr("flp", rlpr_rlpr->filetype) != 0) {
	controlf_add(cf_fd, 'W', rlpr_rlpr->width, 0);
    }

    /*
     * handle any extensions
     */

    switch (component_get_extension()) {

    case R_EXT_HPUX:

	if (rlpr_rlpr->write_back)
	    controlf_add(cf_fd, 'R', rlpr_rlpr->user, 0);

	controlf_add(cf_fd, 'A', rlpr_rlpr->priority, 0);
	controlf_add(cf_fd, 'O', rlpr_rlpr->options, 0);
	break;
    }
}

static int
tmp_file_init(char **cf_path, char **cf_name, char **df_name)
{
    int		cf_fd;

    *cf_path = xmalloc(strlen(rlpr_rlpr->localhost) +
		       strlen(rlpr_rlpr->tmpdir) + sizeof "/cfAxxx");
    sprintf(*cf_path, "%s/cfA%03d", rlpr_rlpr->tmpdir, (int)getpid() % 1000);
    strcat(*cf_path, rlpr_rlpr->localhost);

    *cf_name = strrchr(*cf_path, '/') + 1;
    *df_name = xstrdup(*cf_name);
    *df_name[0] = 'd';

    cf_fd = open(*cf_path, O_RDWR|O_TRUNC|O_CREAT, 0600);
    if (cf_fd == -1) {
	msg(R_ERROR, errno, "tmp_file_init: open: %s", *cf_path);
	free(*df_name);
	free(*cf_path);
    }

    msg(R_DEBUG, 0, "tmp_file_init: cf_fd: %i (%s)", cf_fd, *cf_path);

    unlink(*cf_path);
    return cf_fd;
}

/* ARGSUSED */
static void
tmp_file_next(int cf_fd, char *cf_path, char *cf_name, char *df_name)
{
    cf_name[2]++;		/* cf_path shares the same pointer */
    df_name[2]++;

    lseek(cf_fd, 0, SEEK_SET);
}

static struct option rlpr_opts[] = {
    { "copies",			1, 0, '#'  },
    { "send-data-first",	0, 0, -400 },
    { "help",			0, 0, -401 },
    { "hostname",		1, 0, -402 },
    { "indent",			2, 0, 'i'  },
    { "job",			1, 0, 'J'  },
    { "mail",			0, 0, 'm'  },
    { "no-burst",		0, 0, 'h'  },
    { "remove",			0, 0, 'r'  },
    { "timeout",		1, 0, -403 },
    { "title",			1, 0, 'T'  },
    { "tmpdir",			1, 0, -404 },
    { "user",			1, 0, 'U'  },
    { "version",		0, 0, 'V'  },
    { "width",			2, 0, 'w'  },
    { "windows",		0, 0, -405 },
    { 0,			0, 0,  0   }
};

static const char rlpr_opt_list[] = "1234cdfglnoptvFsC:#:i::J:mhrT:U:w::W:V";
static const char rlpr_hpux_opt_list[] = "o:p:";

struct component comp_rlpr = {
    "rlpr",  rlpr_init, 0,
    { { rlpr_opts, rlpr_opt_list, rlpr_parse_args },
      { 0, rlpr_hpux_opt_list, rlpr_hpux_parse_args }
    }
};
