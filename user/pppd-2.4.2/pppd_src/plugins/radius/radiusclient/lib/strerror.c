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
 * $Id: strerror.c,v 1.2 2002/02/27 15:51:20 dfs Exp $
 *
 * Copyright (C) 1996 Lars Fenneberg and Christian Graefe
 *
 * This file is provided under the terms and conditions of the GNU general
 * public license, version 2 or any later version, incorporated herein by
 * reference.
 *
 */

#include "config.h"
#include "includes.h"

/*
 * if we're missing strerror, these are mostly not defined either
 */
extern int sys_nerr;
extern char *sys_errlist[];

/*
 * Function: strerror
 *
 * Purpose:  implements strerror for systems which lack it. if these
 *			 systems even lack sys_errlist, you loose...
 *
 */


char *strerror(int err)
{
	static char buf[32];

	if (err >= 0 && err < sys_nerr)
		return sys_errlist[err];
	else {
		sprintf(buf, "unknown error: %d", errno);
		return buf;
	}
}
