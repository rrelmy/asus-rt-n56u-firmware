#
# Makefile for libid3tag
#

TOPDIR=../
include $(TOPDIR)Rules.mak

LIBID3TAG=libid3tag.a
LIBID3TAG_SHARED=libid3tag.so
LIBID3TAG_SHARED_FULLNAME=libid3tag-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

# uClibc's libid3tag lacks all aio routines, all clock routines,
# and all shm routines
CSRC= compat.c debug.c file.c frametype.c latin1.c render.c ucs4.c \
      utf8.c version.c crc.c field.c frame.c genre.c parse.c tag.c \
      utf16.c  util.c
OBJS=$(patsubst %.c,%.o, $(CSRC))
CFLAGS += -I../zlib-1.2.3

all: $(LIBID3TAG)

$(LIBID3TAG) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBID3TAG) $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBID3TAG)
	$(INSTALL) -m 644 $(LIBID3TAG) $(TOPDIR)lib/

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBID3TAG_SHARED).$(MAJOR_VERSION) \
		-o $(LIBID3TAG_SHARED_FULLNAME) --whole-archive $(LIBID3TAG) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC) -lz;
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBID3TAG_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBID3TAG_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBID3TAG_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBID3TAG_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBID3TAG_SHARED)
	$(LN) -sf $(LIBID3TAG_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBID3TAG_SHARED).$(MAJOR_VERSION)

clean:
	$(RM) *.[oa] *~ core $(LIBID3TAG_SHARED)* $(LIBID3TAG_SHARED_FULLNAME)*
