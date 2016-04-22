#
# Makefile for epsonps.c
# 
# for Turbo C, use 
#   tcc epsonps.c
# for Microsoft C 5.1, use
#   cl /W3 /DANSI epsonps.c
#
MANDIR = /usr/man/man1
BINDIR = /usr/local/bin

epsonps: epsonps.c
	cc -DANSI epsonps.c -o epsonps

install: epsonps epsonps.pro epsonps.1
	install -c -m 755 epsonps     $(BINDIR)/epsonps
	install -c -m 644 epsonps.pro $(BINDIR)/epsonps.pro
	install -c -m 644 epsonps.1   $(MANDIR)/epsonps.1
	
shar: README epsonps.c epsonps.pro epsonps.1 Makefile
	shar README epsonps.c > epsonps.sh1
	shar epsonps.1 epsonps.pro Makefile > epsonps.sh2
