CC = mipsel-linux-gcc
AR = mipsel-linux-ar
RANLIB = mipsel-linux-ranlib
LIBNAME = libmplus
LIBNAME_STATIC = ${LIBNAME}.a
LIBNAME_SHARED = ${LIBNAME}.so
LIBNAME_MAJOR = $(LIBNAME_SHARED).0

LDFLAGS = -D_ISOC99_SOURCE -std=c99 -lm

SRCS =  mplus.c

LOBJS = $(SRCS:.c=.lo)

.SUFFIXES: .c .lo

all: lib_shared lib_static

.c.lo:
	$(CC) -c -fPIC $(CFLAGS) -o $@ $<

lib_shared: $(LOBJS)
	$(CC) -shared -Wl,-soname,$(LIBNAME_MAJOR) $(LOBJS) $(LDFLAGS) -o $(LIBNAME_SHARED)

lib_static: $(LOBJS)
	$(AR) r $(LIBNAME_STATIC) $(LOBJS)
	$(RANLIB) $(LIBNAME_STATIC)

clean:
	rm -rf *.lo *.so* *.a

#romfs:	lib_shared 
#	install -d $(INSTALLDIR)/lib
#	install -m 755 $(LIBNAME_SHARED) $(INSTALLDIR)/lib
#	cd $(INSTALLDIR)/lib && ln -sf $(LIBNAME_SHARED) $(LIBNAME_MAJOR)
#	$(STRIP) $(INSTALLDIR)/lib/$(LIBNAME_SHARED)

romfs:

