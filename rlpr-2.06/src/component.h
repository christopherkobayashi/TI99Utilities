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

#pragma ident "@(#)component.h	1.1	99/09/16 meem"

#ifndef	COMPONENT_H
#define	COMPONENT_H

#include <sys/types.h>
#include <getopt.h>

/*
 * any program making use of the components framework must
 * somewhere define a program name for use by components.
 */

extern const char	*program_name;
extern const char	*bsd_program_name;

/*
 * extensions are a modular way of handling vendor-extensions to the
 * rlpr suite of programs.  essentially, the component module parses
 * and makes available two pieces of information: the enabled vendor
 * extension (through component_get_extension()) and the various
 * vendor options (passed into a modules parse_args() routine, if it
 * has one for the given vendor.  currently, the only supported
 * vendor extensions are hp/ux.
 */

typedef enum { R_EXT_NONE, R_EXT_HPUX, R_EXT_MAX } rlpr_ext_t;

typedef struct
{
    rlpr_ext_t		type;
    const char	       *name;

} rlpr_ext_item_t;

struct rlpr_comp
{
    char	       *ext_name;
    char	       *ext_args;
    int			ext_argc;
    const char	      **ext_argv;
    rlpr_ext_t		ext_type;
    int			is_initialized;
};

/*
 * routines beginning in `component_' are part of the generic
 * component architecture (i.e, called to initialize the components,
 * etc).  routines beginning in `comp_' are the component system as a
 * component itself.
 */

struct component *	component_init(struct component **, int, char **);
struct component *	component_fini(struct component **);
rlpr_ext_t		component_get_extension(void);

extern struct component comp_component;

struct component_arg_info
{
    struct option	*opts;
    const char		*opt_list;
    int			(*parse_args)(int);
};

struct component
{
    /* generic component data */
    const char			*name;

    /* generic component operations */
    int				(*init)(void);
    int				(*fini)(void);

    /* argument parsing information */
    struct component_arg_info	arg_info[R_EXT_MAX];
};

#define	gen_parse_args		arg_info[R_EXT_NONE].parse_args
#define	gen_opt_list		arg_info[R_EXT_NONE].opt_list
#define	gen_opts		arg_info[R_EXT_NONE].opts

#endif	/* COMPONENT_H */
