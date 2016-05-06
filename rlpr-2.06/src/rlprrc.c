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

#pragma ident "@(#)rlprrc.c	1.3	99/10/29 meem"

#include <string.h>
#include <stdio.h>
#include <lib.h>
#include <errno.h>
#include <stdlib.h>		/* getenv() */

#include "msg.h"
#include "rlprrc.h"
#include "intl.h"

static const char *
rlprrc_search_internal(FILE *rcfile, const char *key, rlpr_search_type_e type)
{
    char	line_buf[R_LINE_LEN];
    char       *queue_pos, *ptr, *line;
    char       *result = 0;
    size_t	keylen = strlen(key);

    msg(R_DEBUG, 0, "rlprrc_search_internal: resolving `%s' (type %d)", key,
	type);

    while (fgets(line_buf, R_LINE_LEN, rcfile) != 0) {

	line = line_buf;
	while (strchr(" \t", *line) != 0)
	    line++;

	/* skip comments or invalid lines */
	if (strchr("#\n", *line) || (queue_pos = strchr(line, ':')) == 0)
	    continue;

	while (strchr(": \t", *queue_pos) != 0)
	    *queue_pos++ = '\0';

	if (type == R_RESOLVE_HOST) {

	    while (strncmp(queue_pos, key, keylen) != 0 ||
		   strchr(" !\n", queue_pos[keylen]) == 0) {
		queue_pos = strpbrk(queue_pos, " \t");
		if (queue_pos == 0)
		    break;
		while (strchr(" \t", *queue_pos) != 0)
		    *queue_pos++ = '\0';
	    }

	    if (queue_pos == 0)
		continue;

	    free(result);
	    result = xstrdup(line);
	    if (queue_pos[keylen] == '!')
		break;

	} else if (type == R_RESOLVE_QUEUE) {

	    if (strcasecmp(line, key) != 0)
		continue;

	    if ((ptr = strpbrk(queue_pos, "\n ")) != 0)
		*ptr = '\0';

	    return xstrdup(queue_pos);
	}
    }

    return result;
}

const char *
rlprrc(const char *key, rlpr_search_type_e type)
{
    FILE       *rcfile = 0;
    char       *home = getenv("HOME");
    char       *confdir = getenv("RLPR_CONFDIR");
    char       *rlprrc_name;
    const char *result = 0;

    if (home != 0) {

	rlprrc_name = xmalloc(strlen(home) + strlen(R_RLPRRC_NAME) + 3);
	sprintf(rlprrc_name, "%s/.%s", home, R_RLPRRC_NAME);

	rcfile = fopen(rlprrc_name, "r");
	free(rlprrc_name);

	if (rcfile != 0)
	    result = rlprrc_search_internal(rcfile, key, type);
	else
	    msg(R_DEBUG, errno, "unable to open ~/.%s", R_RLPRRC_NAME);
    } else
	msg(R_WARNING, 0, "$HOME not set, not using ~/.%s", R_RLPRRC_NAME);

    if (result == 0) {

	if (confdir == 0)
	    confdir = R_CONFDIR;

	rlprrc_name = xmalloc(strlen(confdir) + strlen(R_RLPRRC_NAME) + 3);
	sprintf(rlprrc_name, "%s/%s", confdir, R_RLPRRC_NAME);

	msg(R_DEBUG, 0, "`%s' not found in ~/.%s, checking %s", key,
	    R_RLPRRC_NAME, rlprrc_name);

	rcfile = fopen(rlprrc_name, "r");
	if (rcfile != 0)
	    result = rlprrc_search_internal(rcfile, key, type);
	else
	    msg(R_DEBUG, errno, "unable to open %s", rlprrc_name);

	free(rlprrc_name);
    }

    if (rcfile && fclose(rcfile) == -1)
	msg(R_WARNING, errno, "unable to close %s", R_RLPRRC_NAME);

    msg(R_DEBUG, 0, "rlprrc: `%s' -> `%s' (type %d)", key, result ? result :
	_("<not resolved>"), type);
    return result;
}
