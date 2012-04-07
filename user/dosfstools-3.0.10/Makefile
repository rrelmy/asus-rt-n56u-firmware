# Makefile
#
# Copyright (C) 2008 Daniel Baumann <daniel@debian.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# On Debian systems, the complete text of the GNU General Public License
# can be found in /usr/share/common-licenses/GPL-3 file.

-include $(LINUX_CONFIG)
-include $(CONFIG_CONFIG)
-include $(ARCH_CONFIG)
-include $(MODULES_CONFIG)

INSTALLDIR = $(ROOTDIR)/romfs

OPTFLAGS = -O2 -fomit-frame-pointer -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
#OPTFLAGS = -O2 -fomit-frame-pointer $(shell getconf LFS_CFLAGS)
WARNFLAGS = -Wall
DEBUGFLAGS = -g
CFLAGS += $(OPTFLAGS) $(WARNFLAGS) $(DEBUGFLAGS)

VPATH = src

all: dosfsck

dosfsck: boot.o check.o common.o fat.o file.o io.o lfn.o dosfsck.o

romfs: dosfsck
	$(STRIP) dosfsck
	$(ROMFSINST) /sbin/dosfsck
#	cd $(INSTALLDIR)/sbin && ln -sf dosfsck fsck.msdos
#	cd $(INSTALLDIR)/sbin && ln -sf dosfsck fsck.vfat

clean:
	rm -f *.o
	rm -f dosfsck

