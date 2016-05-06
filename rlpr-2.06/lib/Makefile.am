# Copyright (c) 1998-1999 peter memishian (meem), meem@gnu.org
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# SCCS "%Z%%M%	%I%	%E% meem"

# Process this file with automake to produce Makefile.in

noinst_LIBRARIES = librlpr.a

INCLUDES = -I$(top_srcdir)/intl

#
# This is to workaround an apparent autoconf bug that causes getopt.c
# to not automatically be included in the tarball.
#
EXTRA_DIST = getopt.c

#
# This SOURCES line consists only of .c's which are needed for all
# compilations.  LIBOBJS should expand to a list of objects which
# are needed because of deficiencies on the compilation system.
#

librlpr_a_SOURCES = xmalloc.c xstrtoul.c xstrtol.c xstrdup.c error.c

noinst_HEADERS = error.h lib.h getopt.h xstrtol.h xstrtoul.h

librlpr_a_LIBADD = @LIBOBJS@ @ALLOCA@
librlpr_a_DEPENDENCIES = $(librlpr_a_LIBADD)
