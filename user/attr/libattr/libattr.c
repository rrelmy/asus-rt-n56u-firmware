/*
 * Copyright (c) 2001-2003 Silicon Graphics, Inc.  All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2.1 of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307,
 * USA.
 * 
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 * 
 * http://www.sgi.com 
 * 
 * For further information regarding this notice, see: 
 * 
 * http://oss.sgi.com/projects/GenInfo/SGIGPLNoticeExplan/
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <asm/types.h>

#include <attr/xattr.h>
#include <attr/attributes.h>

#undef MAXNAMELEN
#define MAXNAMELEN 256

/*
 * Convert IRIX API components into Linux/XFS API components
 */
static int
api_convert(char *name, const char *irixname, int irixflags, int compat)
{
	static const char *user_name = "user.";
	static const char *trusted_name = "trusted.";
	static const char *xfsroot_name = "xfsroot.";

	if (strlen(irixname) >= MAXNAMELEN) {
		errno = EINVAL;
		return -1;
	}
	if (irixflags & ATTR_ROOT) {
		if (compat)
			strcpy(name, xfsroot_name);
		else
			strcpy(name, trusted_name);
	} else {
		strcpy(name, user_name);
	}
	strcat(name, irixname);
	return 0;
}

int
attr_get(const char *path, const char *attrname, char *attrvalue,
	 int *valuelength, int flags)
{
	int c, compat;
	char name[MAXNAMELEN+16];

	for (compat = 0; compat < 2; compat++) {
		if ((c = api_convert(name, attrname, flags, compat)) < 0)
			return c;
		if (flags & ATTR_DONTFOLLOW)
			c = lgetxattr(path, name, attrvalue, *valuelength);
		else
			c =  getxattr(path, name, attrvalue, *valuelength);
		if (c < 0 && (errno == ENOATTR || errno == ENOTSUP))
			continue;
		break;
	}
	if (c < 0)
		return c;
	*valuelength = c;
	return 0;
}

int
attr_getf(int fd, const char *attrname, char *attrvalue,
	  int *valuelength, int flags)
{
	int c, compat;
	char name[MAXNAMELEN+16];

	for (compat = 0; compat < 2; compat++) {
		if ((c = api_convert(name, attrname, flags, compat)) < 0)
			return c;
		c = fgetxattr(fd, name, attrvalue, *valuelength);
		if (c < 0 && (errno == ENOATTR || errno == ENOTSUP))
			continue;
		break;
	}
	if (c < 0)
		return c;
	*valuelength = c;
	return 0;
}

int
attr_set(const char *path, const char *attrname, const char *attrvalue,
	 const int valuelength, int flags)
{
	int c, compat, lflags = 0;
	char name[MAXNAMELEN+16];
	void *buffer = (void *)attrvalue;

	if (flags & ATTR_CREATE)
		lflags = XATTR_CREATE;
	else if (flags & ATTR_REPLACE)
		lflags = XATTR_REPLACE;

	for (compat = 0; compat < 2; compat++) {
		if ((c = api_convert(name, attrname, flags, compat)) < 0)
			return c;
		if (flags & ATTR_DONTFOLLOW)
			c = lsetxattr(path, name, buffer, valuelength, lflags);
		else
			c = setxattr(path, name, buffer, valuelength, lflags);
		if (c < 0 && (errno == ENOATTR || errno == ENOTSUP))
			continue;
		break;
	}
	return c;
}

int
attr_setf(int fd, const char *attrname,
	  const char *attrvalue, const int valuelength, int flags)
{
	int c, compat, lflags = 0;
	char name[MAXNAMELEN+16];
	void *buffer = (void *)attrvalue;

	if (flags & ATTR_CREATE)
		lflags = XATTR_CREATE;
	else if (flags & ATTR_REPLACE)
		lflags = XATTR_REPLACE;

	for (compat = 0; compat < 2; compat++) {
		if ((c = api_convert(name, attrname, flags, compat)) < 0)
			return c;
		c = fsetxattr(fd, name, buffer, valuelength, lflags);
		if (c < 0 && (errno == ENOATTR || errno == ENOTSUP))
			continue;
		break;
	}
	return c;
}

int
attr_remove(const char *path, const char *attrname, int flags)
{
	int c, compat;
	char name[MAXNAMELEN+16];

	for (compat = 0; compat < 2; compat++) {
		if ((c = api_convert(name, attrname, flags, compat)) < 0)
			return c;
		if (flags & ATTR_DONTFOLLOW)
			c = lremovexattr(path, name);
		else
			c = removexattr(path, name);
		if (c < 0 && (errno == ENOATTR || errno == ENOTSUP))
			continue;
		break;
	}
	return c;
}

int
attr_removef(int fd, const char *attrname, int flags)
{
	int c, compat;
	char name[MAXNAMELEN+16];

	for (compat = 0; compat < 2; compat++) {
		if ((c = api_convert(name, attrname, flags, compat)) < 0)
			return c;
		c = fremovexattr(fd, name);
		if (c < 0 && (errno == ENOATTR || errno == ENOTSUP))
			continue;
		break;
	}
	return c;
}


/*
 * Helper routines for the attr_multi functions.  In IRIX, the
 * multi routines are a single syscall - in Linux, we break em
 * apart in userspace and make individual syscalls for each.
 */

static int
attr_single(const char *path, attr_multiop_t *op, int flags)
{
	int r = -1;

	errno = -EINVAL;
	flags |= op->am_flags;
	if (op->am_opcode & ATTR_OP_GET)
		r = attr_get(path, op->am_attrname, op->am_attrvalue,
				&op->am_length, flags);
	else if (op->am_opcode & ATTR_OP_SET)
		r = attr_set(path, op->am_attrname, op->am_attrvalue,
				op->am_length, flags);
	else if (op->am_opcode & ATTR_OP_REMOVE)
		r = attr_remove(path, op->am_attrname, flags);
	return r;
}

static int
attr_singlef(const int fd, attr_multiop_t *op, int flags)
{
	int r = -1;

	errno = -EINVAL;
	flags |= op->am_flags;
	if (op->am_opcode & ATTR_OP_GET)
		r = attr_getf(fd, op->am_attrname, op->am_attrvalue,
				&op->am_length, flags);
	else if (op->am_opcode & ATTR_OP_SET)
		r = attr_setf(fd, op->am_attrname, op->am_attrvalue,
				op->am_length, flags);
	else if (op->am_opcode & ATTR_OP_REMOVE)
		r = attr_removef(fd, op->am_attrname, flags);
	return r;
}

/*
 * Operate on multiple attributes of the same object simultaneously
 * 
 * From the manpage: "attr_multi will fail if ... a bit other than
 * ATTR_DONTFOLLOW was set in the flag argument." flags must be
 * checked here as they are not passed into the kernel.
 */
int
attr_multi(const char *path, attr_multiop_t *multiops, int count, int flags)
{
	int i, tmp, r = -1;

	errno = EINVAL;
	if ((flags & ATTR_DONTFOLLOW) != flags)
		return r;

	r = errno = 0;
	for (i = 0; i < count; i++) {
		tmp = attr_single(path, &multiops[i], flags);
		if (tmp) r = tmp;
	}
	return r;
}

int
attr_multif(int fd, attr_multiop_t *multiops, int count, int flags)
{
	int i, tmp, r = -1;

	errno = EINVAL;
	if ((flags & ATTR_DONTFOLLOW) != flags)
		return r;

	r = errno = 0;
	for (i = 0; i < count; i++) {
		tmp = attr_singlef(fd, &multiops[i], flags);
		if (tmp) r = tmp;
	}
	return r;
}
