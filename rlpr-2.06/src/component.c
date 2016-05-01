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

#pragma ident "@(#)component.c	1.2	99/10/29 meem"

#include <sys/types.h>
#include <stddef.h>		/* size_t */
#include <getopt.h>
#include <stdio.h>
#include <lib.h>		/* rlpr utility functions */
#include <string.h>		/* memset() */
#include <stdlib.h>

#include "component.h"
#include "util.h"
#include "msg.h"

static struct rlpr_comp *rlpr_comp;

static int comp_finish_init(void);

/*
 * supported extensions
 */

static const char *ext_to_name[] = { "generic", "hpux" };

static rlpr_ext_item_t ext_table[] = {
    { R_EXT_HPUX,	"hpux"	},
    { R_EXT_HPUX,	"hp/ux" },
    { R_EXT_NONE,	"none"	},
    { R_EXT_NONE,	0	}
};

struct component *
component_init(struct component **component, int argc, char **argv)
{
    unsigned int	i, ext;
    int			c, optcount, arg_count;
    struct option	*full_opts, *opts, *cur_opt;
    char		*full_opt_list, **arg_vector;
    const char		*opt_list;
    size_t		n_opts = 1;		/* for null entry */
    size_t		opt_list_len = 1;	/* for null byte */

    for (i = 0; component[i] != 0; i++) {
	if (component[i]->init != 0) {
	    msg(R_DEBUG, 0, "component `%s': init()", component[i]->name);
	    if (component[i]->init() == 0)
		return component[i];
	}
    }

    msg(R_DEBUG, 0, "component_init: checking for errant arguments");

    /*
     * walk through the option list of each component and size up the
     * requirements for `struct option's and the option list size.
     */

    for (i = 0; component[i] != 0; i++) {

	cur_opt = component[i]->gen_opts;
	for (; cur_opt && cur_opt->name; cur_opt++)
	    n_opts++;

	if (component[i]->gen_opt_list)
	    opt_list_len += strlen(component[i]->gen_opt_list);
    }

    msg(R_DEBUG, 0, "%d struct options, %d opt_list", n_opts, opt_list_len);

    /* allocate the space and fill it in. */

    full_opts		= xmalloc(n_opts * sizeof (struct option));
    full_opt_list	= xmalloc(opt_list_len);

    full_opt_list[0] = '\0';
    memset(full_opts, 0, sizeof (struct option));

    for (i = 0, --n_opts; component[i] != 0; i++) {

	cur_opt = component[i]->gen_opts;
	for (; cur_opt && cur_opt->name; cur_opt++) {
	    full_opts[--n_opts] = *cur_opt;
	}

	if (component[i]->gen_opt_list)
	    strcat(full_opt_list, component[i]->gen_opt_list);
    }

    opterr = 0, optind = 0;
    do {
	c = getopt_long(argc, argv, full_opt_list, full_opts, 0);

	if (c == '?') {
	    if (optopt != 0)
		msg(R_WARNING, 0, "unknown or ambiguous option `-%c'", optopt);
	    else
		msg(R_WARNING, 0, "unknown or ambiguous option `%s'",
		    argv[optind - 1]);
	    continue;
	}

    } while (c != EOF);
    optcount = optind;

    /*
     * now walk through each component in order, calling getopt_long
     * and then passing its parse_args() routine the result.  once our
     * own module is initialized, start calling back any extension
     * parse_args() routines that are available as well.
     */

    for (i = 0; component[i] != 0; i++) {

	for (ext = 0; ext < R_EXT_MAX; ext++) {

	    arg_count  = argc;
	    arg_vector = argv;

	    /*
	     * note that we use `full_opt_list' and `full_opts' here
	     * instead of the component's opt_list and opt fields.
	     * this is necessary since otherwise the argument parser
	     * will get confused (for instance, `-Pfoo' to a component
	     * that doesn't support `-P' will cause the parser to walk
	     * to the `-f' and of `foo' and try passing that in).
	     *
	     * TODO: do this gathering for extension args too.
	     */

	    opt_list   = full_opt_list;
	    opts       = full_opts;

	    if (ext != R_EXT_NONE) {

		if (rlpr_comp->is_initialized && rlpr_comp->ext_type != ext)
		    continue;

		arg_count  = rlpr_comp->ext_argc;
		arg_vector = (char **)rlpr_comp->ext_argv;	/* grrr.. */
		opt_list   = component[i]->arg_info[ext].opt_list;
		opts	   = component[i]->arg_info[ext].opts;
	    }

	    /* if parse_args() is 0 or we have no args, don't bother... */
	    if (component[i]->arg_info[ext].parse_args == 0 ||
		arg_count == 0 || arg_vector == 0)
		continue;

	    msg(R_DEBUG, 0, "component `%s': parse_args() for type `%s'",
		component[i]->name, ext_to_name[ext]);

	    optind = 0;
	    do {
		c = getopt_long(arg_count, arg_vector, opt_list, opts, 0);

		if (component[i]->arg_info[ext].parse_args(c) == 0)
		    return component[i];

	    } while (c != EOF);
	}
    }

    free(full_opt_list);
    free(full_opts);

    optind = optcount;
    return 0;
}

struct component *
component_fini(struct component **component)
{
    unsigned int	i;

    for (i = 0; component[i] != 0; i++) {

	if (component[i]->fini != 0) {
	    msg(R_DEBUG, 0, "component `%s': fini()", component[i]->name);
	    if (component[i]->fini() == 0)
		return component[i];
	}
    }

    return 0;
}

rlpr_ext_t
component_get_extension(void)
{
    if (rlpr_comp->is_initialized == 0)
	return R_EXT_NONE;

    return rlpr_comp->ext_type;
}

static int
comp_init(void)
{
    rlpr_comp = xmalloc(sizeof (struct rlpr_comp));

    rlpr_comp->ext_name = getenv("RLPR_EXTENSION");
    rlpr_comp->ext_args = getenv("RLPR_EXTARGS");
    rlpr_comp->ext_type = R_EXT_NONE;
    rlpr_comp->ext_argv = 0;
    rlpr_comp->ext_argc = 0;

    return 1;
}

static int
comp_parse_args(int opt)
{
    switch (opt) {

    case 'A':
	rlpr_comp->ext_args = optarg;
	break;

    case 'E':
	rlpr_comp->ext_name = optarg;
	break;

    case EOF:
	return comp_finish_init();
    }

    return 1;
}

static int
comp_finish_init(void)
{
    unsigned int	i;
    const char		**argv = 0;

    if (rlpr_comp->ext_name == 0)
	msg(R_DEBUG, 0, "no extensions provided");

    else {

	for (i = 0; ext_table[i].name != 0; i++) {
	    if (strcasecmp(rlpr_comp->ext_name, ext_table[i].name) == 0) {
		rlpr_comp->ext_type = ext_table[i].type;
		break;
	    }
	}

	if (ext_table[i].name == 0)
	    msg(R_WARNING, 0, "unsupported extension `%s', ignoring...",
		rlpr_comp->ext_name);
	else
	    msg(R_DEBUG, 0, "using extension `%s'", rlpr_comp->ext_name);
    }

    if (rlpr_comp->ext_args != 0) {
	argv = string_to_args(rlpr_comp->ext_args, &rlpr_comp->ext_argc);
	if (argv == 0)
	    rlpr_comp->ext_argc = 0;
	rlpr_comp->ext_argv = argv;
    }

    rlpr_comp->is_initialized = 1;

    return 1;
}

static int
comp_fini(void)
{
    rlpr_comp->is_initialized = 0;
    free(rlpr_comp->ext_argv);
    free(rlpr_comp);

    return 1;
}

static struct option comp_opts[] = {
    { "extension",	1, 0, 'E' },
    { "ext",		1, 0, 'E' },
    { "extargs",	1, 0, 'A' },
    { 0,		0, 0,  0  }
};

static const char comp_opt_list[] = "E:A:";

struct component comp_component = {
    "component", comp_init, comp_fini,
    { { comp_opts, comp_opt_list, comp_parse_args } }
};
