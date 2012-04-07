
BIN = xfrtest
INSTALLDIR = $(ROOTDIR)/romfs

TOP = $(ROOTDIR)/user

CFLAGS  += -I. -I$(TOP)/shared -I$(TOP)/shared/include -I$(ROOTDIR)/linux-2.6.21.x/include
CFLAGS  += -Wall -s -O2 -DASUS_NVRAM
CFLAGS  += -I$(TOP)/nvram -I$(TOP)/libdisk


LDFLAGS += -L$(TOP)/shared -lshared -s

all: ${BIN}
	${STRIP} ${BIN}

crc_gen: xfrtest.c
	gcc -o xfrtest xfrtest.c

romfs:
	$(ROMFSINST) /usr/sbin/$(BIN)

clean:
	-rm -f *.o xfrtest a.out
