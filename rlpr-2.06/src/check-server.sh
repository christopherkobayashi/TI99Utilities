#! /bin/sh
#
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
# SCCS "@(#)check-server.sh	1.2	99/10/29 meem"

#
# check-server.sh: diagnose common problems on the lpd server side.
#

if [ $# -lt 2 ]; then
    echo "usage: $0 client-ip-address local-printer [proxy-ip-address]"
    exit 1
fi

RETVAL=0
INCOMING_HOST=${3:-$1}
LOCAL_PRINTER=$2

#
# verify lpd is running (we don't go looking for the process name
# since we're not sure what it will be called or even if it will
# be there at all [it might be started by inetd]).
#

telnet 127.0.0.1 515 2>&1 | grep -i "refused" > /dev/null 2>&1
if [ $? -ne 1 ]; then
    echo "$0: lpd does not appear to be currently running."
    RETVAL=1
fi

#
# check that the printer exists
#

for FILE in /etc/printcap /etc/printers.conf; do 
    if [ -f $FILE ]; then
	grep '\(^[ 	]*\||\)'$LOCAL_PRINTER'[:|]' $FILE > /dev/null 2&>1
	if [ $? -ne 0 ]; then
	    echo "$0: no entry for printer \`$LOCAL_PRINTER' in $FILE"
	    RETVAL=1
	fi
    fi
done

#
# verify the incoming host is known and visible to the server.
#

FOUND=0
for FILE in /etc/hosts.equiv /etc/hosts.lpd; do
    if [ -f $FILE ]; then
	grep $INCOMING_HOST $FILE > /dev/null 2>&1 
	if [ $? -eq 0 ]; then
	    FOUND=$FILE
	fi
    fi
done

if [ $FOUND = "0" ]; then
    echo "$0: no entry for \`$INCOMING_HOST' in /etc/hosts.lpd or /etc/hosts.equiv"
    RETVAL=1
fi

exit $RETVAL
