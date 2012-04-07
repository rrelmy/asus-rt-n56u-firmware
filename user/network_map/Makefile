EXEC = networkmap
OBJS = function.o networkmap.o

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

CFLAGS  += -Wall -DASUS -DASUS_NVRAM -DBCMNVRAM -I. -I$(SHDIR)/include -I$(ROOTDIR)/linux-2.6.21.x/include 
CFLAGS  += -s -O2 -DNO_PARALLEL
LDFLAGS += -L$(SHDIR) -L$(USERDIR)/netconf -lshared -lnetconf

#CFLAGS	+= -DDEBUG -DDEBUG_MORE

all: $(EXEC) #arpstorm

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

#arpstorm: shutils.o arpstorm.o
#	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

romfs:
	$(ROMFSINST) /usr/sbin/$(EXEC)
#	$(ROMFSINST) /usr/sbin/arpstorm

clean: 
	rm -rf *.o $(EXEC) *~
