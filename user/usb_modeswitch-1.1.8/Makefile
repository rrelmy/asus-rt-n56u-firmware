LIBDIR = $(ROOTDIR)/lib/libusb-0.1.12
USERDIR = $(ROOTDIR)/user
CFLAGS += -I$(LIBDIR) -DASUS_NVRAM -I$(USERDIR)/shared/include -I$(ROOTDIR)/linux-2.6.21.x/include 
LDLIBS += -L$(LIBDIR) -lusb -L$(USERDIR)/shared -lshared
EXEC = usb_modeswitch
SRCS = usb_modeswitch.c

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	#$(ROMFSINST) conf /etc_ro/usb

clean:
	-rm -f $(EXEC) *.o

