#
# Copyright (c) 2000-2003 Silicon Graphics, Inc.  All Rights Reserved.
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
# 
# This program is distributed in the hope that it would be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# 
# Further, this software is distributed without any warranty that it is
# free of the rightful claim of any third person regarding infringement
# or the like.  Any license provided herein, whether implied or
# otherwise, applies only to this software file.  Patent licenses, if
# any, provided herein do not apply to combinations of this program with
# other software, or any other product whatsoever.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write the Free Software Foundation, Inc., 59
# Temple Place - Suite 330, Boston MA 02111-1307, USA.
# 
# Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
# Mountain View, CA  94043, or:
# 
# http://www.sgi.com 
# 
# For further information regarding this notice, see: 
# 
# http://oss.sgi.com/projects/GenInfo/SGIGPLNoticeExplan/
#

TOPDIR = .
HAVE_BUILDDEFS = $(shell test -f $(TOPDIR)/include/builddefs && echo yes || echo no)

ifeq ($(HAVE_BUILDDEFS), yes)
include $(TOPDIR)/include/builddefs
endif

CONFIGURE = configure include/builddefs include/config.h
LSRCFILES = configure configure.in aclocal.m4 Makepkgs install-sh exports \
	README VERSION

LDIRT = config.log .dep config.status config.cache confdefs.h conftest* \
	Logs/* built .census install.* install-dev.* install-lib.* *.gz

SUBDIRS = include libmisc libattr attr getfattr setfattr \
	  examples test m4 man doc po debian build

default: $(CONFIGURE)
ifeq ($(HAVE_BUILDDEFS), no)
	$(MAKE) -C . $@
else
	$(SUBDIRS_MAKERULE)
endif

ifeq ($(HAVE_BUILDDEFS), yes)
include $(BUILDRULES)
else
clean:	# if configure hasn't run, nothing to clean
endif

$(CONFIGURE):
	autoconf
	./configure \
		--prefix=/ \
		--exec-prefix=/ \
		--sbindir=/bin \
		--bindir=/usr/bin \
		--libdir=/lib \
		--libexecdir=/usr/lib \
		--includedir=/usr/include \
		--mandir=/usr/share/man \
		--datadir=/usr/share \
		$$LOCAL_CONFIGURE_OPTIONS
	touch .census

aclocal.m4::
	aclocal --acdir=$(TOPDIR)/m4 --output=$@

install: default
	$(SUBDIRS_MAKERULE)

romfs:
#	mv attr attr_tmp
#	cp attr_tmp/.libs/attr ./
#	$(STRIP) attr
#	$(ROMFSINST) /bin/attr
#	rm -f attr
#	mv attr_tmp attr
#	mv getfattr getfattr_tmp
#	cp getfattr_tmp/.libs/getfattr ./
#	$(STRIP) getfattr
#	$(ROMFSINST) /bin/getfattr
#	rm -f getfattr
#	mv getfattr_tmp getfattr
#	mv setfattr setfattr_tmp
#	cp setfattr_tmp/.libs/setfattr ./
#	$(STRIP) setfattr
#	$(ROMFSINST) /bin/setfattr
#	rm -f setfattr
#	mv setfattr_tmp setfattr
	cp libattr/.libs/libattr.so.1.1.0 libattr.so.1
	$(STRIP) libattr.so.1
	$(ROMFSINST) /lib/libattr.so.1
	rm -f libattr.so.1
	cd $(INSTALLDIR)/lib && ln -sf libattr.so.1 libattr.so


install-dev install-lib: default
	$(SUBDIRS_MAKERULE)

realclean distclean: clean
	rm -f $(LDIRT) $(CONFIGURE)
	rm -rf autom4te.cache Logs
