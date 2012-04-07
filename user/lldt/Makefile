VPATH = src
CFLAGS += -O3 -pipe -mips32 -mtune=mips32 -Wall -Wno-unused -s -fpack-struct -Wpadded  -D__LITTLE_ENDIAN
LD= $(CC)
OS_LAYER = osl-linux.c

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user
CFLAGS  += -Wall -DASUS_NVRAM -I. -I$(SHDIR)/include -I$(ROOTDIR)/linux-2.6.21.x/include
LDFLAGS += -L$(SHDIR) -lshared

all: lld2d lld2test

include src/common.mk

depend:
	$(CC) $(CFLAGS) -M $(DCFILES) >.depend

clear:
	rm -f *.o lld2d lld2test

-include .depend
