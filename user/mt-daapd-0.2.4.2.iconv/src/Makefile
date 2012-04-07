EXEC = mt-daapd
OBJS = configfile.o main.o mp3-scanner.o query.o \
       strcasestr.o daap.o dynamic-art.o mDNS.o \
       redblack.o strsep.o daap-proto.o err.o mDNSPosix.o \
       parser.o rend-unix.o rend-posix.o uici.o lexer.o mDNSUNP.o\
       playlist.o restart.o webserver.o db-gdbm.o

CFLAGS += -DWITH_GDBM 
CFLAGS += -DNOT_HAVE_SA_LEN -DSETPGRP_VOID -DVERSION=\"0.2.4.2\" -DPACKAGE=\"mt-daapd\" -I./
CFLAGS += -I$(ROOTDIR)/lib/libgdbm-1.8.3 -I$(ROOTDIR)/lib/libid3tag-0.15.1b
CFLAGS += -DCODEPAGE_CONVERSION
LDLIBS = -lgdbm -lid3tag -lz -lpthread
LDLIBS += -L../../extra_bin -liconv

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user
CFLAGS += -DASUS_NVRAM -I$(ROOTDIR)/linux-2.6.21.x/include
LDLIBS += -L$(SHDIR) -lshared

all: $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)
#	$(STRIP) libiconv.so.0.0.0
#	$(ROMFSINST) /lib/libiconv.so.0.0.0
#	cd $(INSTALLDIR)/lib && ln -sf libiconv.so.0.0.0 libiconv.so.0
#	cd $(INSTALLDIR)/lib && ln -sf libiconv.so.0.0.0 libiconv.so

clean:
	rm -f $(EXEC) *.o
