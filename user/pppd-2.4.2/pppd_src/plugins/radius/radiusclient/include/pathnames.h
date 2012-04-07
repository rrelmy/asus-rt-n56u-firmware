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
 * $Id: pathnames.h,v 1.2 2002/02/27 15:51:19 dfs Exp $
 *
 * Copyright (C) 1995,1996 Lars Fenneberg
 *
 * Copyright 1992 Livingston Enterprises, Inc.
 *
 * Copyright 1992,1993, 1994,1995 The Regents of the University of Michigan
 * and Merit Network, Inc. All Rights Reserved
 *
 * See the file COPYRIGHT for the respective terms and conditions.
 * If the file is missing contact me at lf@elemental.net
 * and I'll send you a copy.
 *
 */

#ifndef PATHNAMES_H
#define PATHNAMES_H

#define _PATH_DEV_URANDOM	"/dev/urandom"		/* Linux only */
#define _PATH_ETC_ISSUE		"/etc/issue"

/* normally defined in the Makefile */
#ifndef _PATH_ETC_RADIUSCLIENT_CONF
#define _PATH_ETC_RADIUSCLIENT_CONF       "/etc/radiusclient.conf"
#endif

#endif /* PATHNAMES_H */
