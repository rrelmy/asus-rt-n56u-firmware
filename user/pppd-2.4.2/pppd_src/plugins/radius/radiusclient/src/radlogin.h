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
 * $Id: radlogin.h,v 1.1.1.1 2007/02/15 12:14:18 jiahao Exp $
 *
 * Copyright (C) 1996 Lars Fenneberg
 *
 * See the file COPYRIGHT for the respective terms and conditions. 
 * If the file is missing contact me at lf@elemental.net 
 * and I'll send you a copy.
 *
 */

#ifndef RADLOGIN_H
#define RADLOGIN_H

#undef __P
#if defined (__STDC__) || defined (_AIX) || (defined (__mips) && defined (_SYSTYPE_SVR4)) || defined(WIN32) || defined(__cplusplus)
# define __P(protos) protos
#else
# define __P(protos) ()
#endif

typedef void (*LFUNC)(char *);

/* radius.c */
LFUNC auth_radius(UINT4, char *, char *);
void radius_login(char *);

/* local.c */
LFUNC auth_local __P((char *, char *));
void local_login __P((char *));

#endif /* RADLOGIN_H */
