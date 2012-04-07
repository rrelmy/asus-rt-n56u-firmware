#
# Makefile for libgdbm
#

TOPDIR=../
include $(TOPDIR)Rules.mak

LIBGDBM=libgdbm.a
LIBGDBM_SHARED=libgdbm.so
LIBGDBM_SHARED_FULLNAME=libgdbm-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

# uClibc's libgdbm lacks all aio routines, all clock routines,
# and all shm routines
CSRC= bucket.c dbmdirfno.c dbmpagfno.c delete.c gdbmclose.c gdbmfdesc.c  \
      gdbmseq.c global.c update.c close.c dbmfetch.c dbmrdonly.c falloc.c \
      gdbmdelete.c gdbmfetch.c gdbmsetopt.c hash.c version.c \
      dbmclose.c dbminit.c dbmseq.c fetch.c gdbmerrno.c gdbmopen.c \
      gdbmstore.c seq.c dbmdelete.c dbmopen.c dbmstore.c findkey.c \
      gdbmexists.c gdbmreorg.c gdbmsync.c store.c

OBJS=$(patsubst %.c,%.o, $(CSRC))

all: $(LIBGDBM)

$(LIBGDBM) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBGDBM) $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBGDBM)
	$(INSTALL) -m 644 $(LIBGDBM) $(TOPDIR)lib/

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBGDBM_SHARED).$(MAJOR_VERSION) \
		-o $(LIBGDBM_SHARED_FULLNAME) --whole-archive $(LIBGDBM) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC);
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBGDBM_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBGDBM_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBGDBM_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBGDBM_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBGDBM_SHARED)
	$(LN) -sf $(LIBGDBM_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBGDBM_SHARED).$(MAJOR_VERSION)

clean:
	$(RM) *.[oa] *~ core $(LIBGDBM_SHARED)* $(LIBGDBM_SHARED_FULLNAME)*
