#
# U2EC Makefile
#
# Copyright (C) 2008 ASUSTeK Corporation
#
#

#ifeq ($(strip $(CROSS_COMPILE)),)
#CROSS_COMPILE := mipsel-linux-
TOP := $(shell pwd)/..
SRCBASE := $(TOP)/..
#INSTALLDIR := $(TOP)/mipsel/install
INSTALLDIR := $(ROOTDIR)/romfs
#CC := $(CROSS_COMPILE)gcc
#AR := $(CROSS_COMPILE)ar
#AS := $(CROSS_COMPILE)as
#LD := $(CROSS_COMPILE)ld
#NM := $(CROSS_COMPILE)nm
#RANLIB := $(CROSS_COMPILE)ranlib
#STRIP := $(CROSS_COMPILE)strip
#SIZE := $(CROSS_COMPILE)size
#endif

PROGS	= u2ec
OBJS	= usbsock.o testusb.o
DEBUG	= n

ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DU2EC_DEBUG
	DEBFLAGS += -DPDEBUG_SENDSECV
	DEBFLAGS += -DPDEBUG_DECODE
	OBJS += debug.o decode.o
else
	DEBFLAGS = -s -O2
endif

#ifeq ($(strip $(CROSS_COMPILE)),)
#	CC = gcc
#	DEBFLAGS += -DU2EC_ONPC
#	LDFLAGS = -L/usr/local/lib -lusb -lpthread -Wl,--rpath -Wl,/usr/local/lib
#else
	CFLAGS = -DSUPPORT_LPRng
	CFLAGS += -I$(TOP)/shared
	CFLAGS += -I$(SRCBASE)/linux-2.6.21.x/include -I$(SRCBASE)/linux-2.6.21.x/include/nvram
	LDFLAGS = -lpthread
	LDFLAGS += -L$(SRCBASE)/lib/libusb-0.1.12 -L$(INSTALLDIR)/usr/lib -lusb
	LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/usr/lib -lshared
#	LDFLAGS += -L$(TOP)/nvram -L$(INSTALLDIR)/usr/lib -lnvram
#endif

CFLAGS += -Wall -I.
CFLAGS += $(DEBFLAGS)

all: $(PROGS)
	$(STRIP) $(PROGS)

$(PROGS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

#ifneq ($(strip $(CROSS_COMPILE)),)
install: $(PROGS)
	install -D $(PROGS) $(INSTALLDIR)/usr/sbin/$(PROGS)
#	$(STRIP) $(INSTALLDIR)/usr/sbin/$(PROGS)
romfs:
	$(ROMFSINST) /usr/sbin/$(PROGS)
#endif

clean:
	rm -f $(PROGS) *.o *~
