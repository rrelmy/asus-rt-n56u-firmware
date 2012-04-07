#
# Copyright (c) 2000-2002 Silicon Graphics, Inc.  All Rights Reserved.
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of version 2.1 of the GNU Lesser General Public License
# as published by the Free Software Foundation.
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
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307,
# USA.
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

TOPDIR = ..

LTLDFLAGS = -Wl,--version-script,$(TOPDIR)/exports
include $(TOPDIR)/include/builddefs

LTLIBRARY = libattr.la
LT_CURRENT = 2
LT_REVISION = 0
LT_AGE = 1

CFILES = libattr.c syscalls.c attr_copy_fd.c attr_copy_file.c attr_copy_check.c
HFILES = libattr.h

LCFLAGS = -include libattr.h

default: $(LTLIBRARY)

include $(BUILDRULES)

install: default
	$(INSTALL_LTLIB_DEV)

install-lib: default
	$(INSTALL_LTLIB)

install-dev: default
	$(INSTALL_LTLIB_DEV)
