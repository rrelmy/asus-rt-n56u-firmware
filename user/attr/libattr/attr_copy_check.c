/* Copy extended attributes between files - default check callback */
 
/* Copyright (C) 2003 Andreas Gruenbacher <agruen@suse.de>, SuSE Linux AG.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#include <string.h>
#include "error_context.h"

int
attr_copy_check_permissions(const char *name, struct error_context *ctx)
{
	/* Skip POSIX ACLs. */
	if (strncmp(name, "system.posix_acl_", 17) == 0 &&
	    (strcmp(name+17, "access") == 0 ||
	     strcmp(name+17, "default") == 0))
		return 0;

	/* Skip permissions attributes which are used on IRIX, and
	   hence are part of the XFS ondisk format (incl. ACLs). */
	if (strncmp(name, "trusted.SGI_", 12) == 0 &&
	    (strcmp(name+12, "ACL_DEFAULT") == 0 ||
	     strcmp(name+12, "ACL_FILE") == 0 ||
	     strcmp(name+12, "CAP_FILE") == 0 ||
	     strcmp(name+12, "MAC_FILE") == 0))
		return 0;

	/* The xfsroot namespace mirrored attributes, some of which
	   are also also available via the system.* and trusted.*
	   namespaces.  To avoid the problems this would cause,
	   we skip xfsroot altogether.
	   Note: xfsroot namespace has now been removed from XFS. */
	if (strncmp(name, "xfsroot.", 8) == 0)
		return 0;

	return 1;
}

