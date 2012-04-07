# Generated automatically from Makefile.in by configure.
#***********************************************************************
#
# Makefile
#
# Makefile for Roaring Penguin's Linux PPPoE plugin.
#
# Copyright (C) 2001 Roaring Penguin Software Inc.
#
# This program may be distributed according to the terms of the GNU
# General Public License, version 2 or (at your option) any later version.
#
# $Id: Makefile,v 1.1.1.1 2007/02/15 12:14:18 jiahao Exp $
#***********************************************************************

# Version is set ONLY IN THE MAKEFILE!  Don't delete this!
VERSION=3.3

COPTS=-O2 -g
CFLAGS= $(COPTS) -I.. -I../../include -I$(ROOTDIR)/lib/include -fPIC
all: pptp.so

pptp.so: libplugin.a pptp.o
	$(LD) -o pptp.so -shared pptp.o libplugin.a

install: all
	$(INSTALL) -d -m 755 $(LIBDIR)
	$(INSTALL) -s -c -m 4550 pptp.so $(LIBDIR)

clean:
	rm -f *.o *.so *.a

pptp.o: pptp.c
	$(CC) '-DRP_VERSION="$(VERSION)"' $(CFLAGS) -I../../..  -c -o pptp.o -fPIC pptp.c

libplugin.a: dirutil.o orckit_quirks.o pptp_callmgr.o pptp_ctrl.o pptp_quirks.o util.o vector.o
	$(AR) -rc $@ $^

vector.o: vector.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o vector.o -fPIC vector.c

util.o: util.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o util.o -fPIC util.c

pptp_quirks.o: pptp_quirks.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o pptp_quirks.o -fPIC pptp_quirks.c

pptp_ctrl.o: pptp_ctrl.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o pptp_ctrl.o -fPIC pptp_ctrl.c

pptp_callmgr.o: pptp_callmgr.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o pptp_callmgr.o -fPIC pptp_callmgr.c

orckit_quirks.o: orckit_quirks.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o orckit_quirks.o -fPIC orckit_quirks.c

dirutil.o: dirutil.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o dirutil.o -fPIC dirutil.c

