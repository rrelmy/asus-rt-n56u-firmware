EXEC = networkmap
OBJS = function.o networkmap.o

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

CFLAGS  += -Wall -DASUS -DASUS_NVRAM -DBCMNVRAM -I. -I$(SHDIR)/include -I$(SHDIR)/ -I$(ROOTDIR)/linux-2.6.21.x/include 
CFLAGS  += -s -O2 -DNO_PARALLEL
LDFLAGS += -L$(SHDIR) -L$(USERDIR)/netconf -lshared -lnetconf

#CFLAGS  += -DDEBUG -DDEBUG_MORE

all: networkmap

networkmap: function.o networkmap.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

ipscan: function.o ipscan.o 
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

ctrlpt: ctrlpt.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

upnp:   upnp.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

install: all
	install -D networkmap ${INSTALLDIR}/usr/sbin/networkmap

romfs:
	$(ROMFSINST) /usr/sbin/$(EXEC)

clean: 
	rm -rf *.o networkmap arpstorm
