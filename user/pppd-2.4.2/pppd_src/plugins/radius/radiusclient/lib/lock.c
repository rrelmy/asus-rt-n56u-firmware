/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * $Id: lock.c,v 1.2 2002/02/27 15:51:20 dfs Exp $
 *
 * Copyright (C) 1997 Lars Fenneberg
 *
 * See the file COPYRIGHT for the respective terms and conditions.
 * If the file is missing contact me at lf@elemental.net
 * and I'll send you a copy.
 *
 */

#include "config.h"
#include "includes.h"

#if defined(HAVE_FLOCK)

int do_lock_exclusive(int fd)
{
	return flock(fd, LOCK_EX|LOCK_NB);
}

int do_unlock(int fd)
{
	return flock(fd, LOCK_UN);
}

#elif defined(HAVE_FCNTL)

int do_lock_exclusive(int fd)
{
	flock_t fl;
	int res;

	memset((void *)&fl, 0, sizeof(fl));

	fl.l_type = F_WRLCK;
	fl.l_whence = fl.l_start = 0;
	fl.l_len = 0; /* 0 means "to end of file" */

	res = fcntl(fd, F_SETLK, &fl);

	if ((res == -1) && (errno == EAGAIN))
		errno = EWOULDBLOCK;

	return res;
}

int do_unlock(int fd)
{
	flock_t fl;

	memset((void *)&fl, 0, sizeof(fl));

	fl.l_type = F_UNLCK;
	fl.l_whence = fl.l_start = 0;
	fl.l_len = 0; /* 0 means "to end of file" */

	return fcntl(fd, F_SETLK, &fl);
}

#else
YOU_LOOSE "need either flock(2) or fcntl(2)"
#endif
