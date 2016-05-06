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

CLEANFILES = intl/libintl.h
EXTRA_DIST = INSTALL.generic $(PACKAGE)-$(VERSION).lsm config.rpath
SUBDIRS = po intl lib man src

bsdlinks:
	cd src && $(MAKE) bsdlinks

install-setuid: all
	cd src && $(MAKE) install-setuid

install-proxy: all
	cd src && $(MAKE) install-proxy
