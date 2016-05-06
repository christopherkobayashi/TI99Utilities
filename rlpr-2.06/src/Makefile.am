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
# SCCS "@(#)Makefile.am	1.5	01/01/01 meem"

# Process this file with automake to produce Makefile.in

localedir	= $(datadir)/locale
LDFLAGS		= -L../lib
LDADD		= @INTLLIBS@ ../lib/librlpr.a
INCLUDES	= -I$(top_srcdir)/intl -I$(top_srcdir)/lib \
		  -DLOCALEDIR=\"$(localedir)\"
bin_PROGRAMS	= rlpr rlpq rlprm rlprd
rlpr_SOURCES	= rlpr.c component.c msg.c rlprrc.c util.c client.c
rlpq_SOURCES	= rlpq.c component.c msg.c rlprrc.c util.c client.c
rlprm_SOURCES	= rlprm.c component.c msg.c rlprrc.c util.c client.c
rlprd_SOURCES	= rlprd.c component.c msg.c util.c

EXTRA_DIST	= check-server.sh

noinst_HEADERS	= client.h component.h intl.h msg.h rfc1179.h rlpq.h rlpr.h \
		  rlprd.h rlprm.h rlprrc.h util.h

#
# Special directive for making links.  This is kind of a hack;
# it appears that Automake doesn't mind if we add stuff that we
# want to end up in the real Makefile in here as long as we
# avoid its variable names.
#
link_programs	= rlpr rlpq rlprm

install-setuid:	install
	@echo "setting rlpr, rlpq, rlprm and rlprd to be setuid root..."
	@chown root $(bindir)/rlpr
	@chmod 4755 $(bindir)/rlpr
	@chown root $(bindir)/rlpq
	@chmod 4755 $(bindir)/rlpq
	@chown root $(bindir)/rlprm
	@chmod 4755 $(bindir)/rlprm
	@chown root $(bindir)/rlprd
	@chmod 4755 $(bindir)/rlprd

install-proxy: install
	@echo "setting rlprd to be setuid root..."
	@chown root $(bindir)/rlprd
	@chmod 4755 $(bindir)/rlprd

bsdlinks:	$(link_programs)
	@echo "installing bsd symlinks..."
	-mv /usr/bin/lpr /usr/bin/lpr.bsd
	ln -s $(bindir)/rlpr /usr/bin/lpr
	-mv /usr/bin/lpq /usr/bin/lpq.bsd
	ln -s $(bindir)/rlpq /usr/bin/lpq
	-mv /usr/bin/lprm /usr/bin/lprm.bsd
	ln -s $(bindir)/rlprm /usr/bin/lprm


