#
# Linux netconf Makefile
#
# Copyright 2004, ASUSTeK Inc.
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.1.1.1 2007/02/15 12:14:15 jiahao Exp $
#

CFLAGS	= -I. -I$(ROOTDIR)/linux-2.6.21.x/include -I../shared/include -I$(ROOTDIR)/user/shared -I$(ROOTDIR)/user/iptables/include -I$(ROOTDIR)/user/iptables/libiptc
CFLAGS	+= -DASUS_NVRAM -s
#LDFLAGS += -L. -L$(ROOTDIR)/user/iptables/libiptc
#ifeq ($(CONFIG_LINUXDIR), linux-2.6.21.x)
LDFLAGS = -L. -L$(ROOTDIR)/user/iptables/libiptc
#else #linux 2.4
#LDFLAGS = -L. -L$(ROOTDIR)/user/iptables/libiptc
#endif


all: libnetconf.so netconf

romfs:
	$(STRIP) libnetconf.so
	$(ROMFSINST) /lib/libnetconf.so
#	$(STRIP) netconf
#	$(ROMFSINST) /bin/netconf

clean:
	rm -f *.o *.a *.so netconf

libnetconf.so: netconf_linux.o
	$(LD) -shared -o $@ $^ $(LDFLAGS) -liptc

netconf: main.o
	$(CC) -o $@ $< $(LDFLAGS) -lnetconf
