LIB = libdetectWAN.so
OBJS = detectWAN.o

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

CFLAGS  += -Wall -DASUS_NVRAM -I. -I$(SHDIR)/include -I$(ROOTDIR)/linux-2.6.21.x/include -I$(SHDIR)
LDFLAGS += -L$(SHDIR) -lshared

all: $(LIB) detectWan	

$(LIB): $(OBJS)
	$(CC) -shared -o $@ $^ $(CFLAGS) 
	$(STRIP) $(LIB)
	
detectWan: detectWAN.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
	$(STRIP) $@

romfs:
	$(ROMFSINST) /lib/$(LIB)
	$(ROMFSINST) /sbin/detectWan
	
.PHONY: clean
clean:
	rm -f detectWan *~ *.o *.so 
