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
# SCCS "@(#)Makefile.am	1.1	99/09/16 meem"

# Process this file with automake to produce Makefile.in

man_MANS = rlpr.1 rlpq.1 rlprm.1 rlprd.8 rlprrc.5
#
# For some reason, DISTFILES as built by automake does not include
# $(MANS) in its definition; other programs (like fileutils) do.
# XXX what's going on?
#
EXTRA_DIST = $(MANS)
