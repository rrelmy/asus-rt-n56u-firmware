# Generated automatically from Makefile.in by configure.
# Generated automatically from Makefile.in by configure.
# $Id: Makefile.in,v 1.2 2002/09/30 19:45:00 dskoll Exp $
#
# Makefile for event-handling library
#
# Copyright 2002 Roaring Penguin Software Inc.
#
# This software may be distributed according to the terms of the GNU
# General Public License, version 2 or (at your option) any later version.
# LIC: GPL

OBJS=event.o event_tcp.o hash.o event_sig.o
SRCS=$(OBJS:.o=.c)
HDRS=event.h event_tcp.h eventpriv.h hash.h
CFLAGS=-g -O2 -I..

all: libevent.a

libevent.a: $(OBJS)
	rm -f libevent.a
	#mipsel-uclibc-ar -cq libevent.a $(OBJS)
	#mipsel-uclibc-ranlib libevent.a
	$(CROSS)ar -cq libevent.a $(OBJS)
	$(CROSS)ranlib libevent.a

event.o: event.c $(HDRS)
	#mipsel-uclibc-gcc $(CFLAGS) -c -o event.o event.c
	$(CC) $(CFLAGS) -c -o event.o event.c

hash.o: hash.c $(HDRS)
	#mipsel-uclibc-gcc $(CFLAGS) -c -o hash.o hash.c
	$(CC) $(CFLAGS) -c -o hash.o hash.c

event_sig.o: event_sig.c $(HDRS)
	#mipsel-uclibc-gcc $(CFLAGS) -c -o event_sig.o event_sig.c
	$(CC) $(CFLAGS) -c -o event_sig.o event_sig.c

event_tcp.o: event_tcp.c $(HDRS)
	#mipsel-uclibc-gcc $(CFLAGS) -c -o event_tcp.o event_tcp.c
	$(CC) $(CFLAGS) -c -o event_tcp.o event_tcp.c

clean: FORCE
	rm -f *.a *.o *~

FORCE:

.phony: FORCE
