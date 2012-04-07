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
 * define path names
 *
 * $Id: pathnames.h,v 1.15 2002/11/02 19:48:13 carlsonj Exp $
 */

#ifdef HAVE_PATHS_H
#include <paths.h>

#else /* HAVE_PATHS_H */
#ifndef _PATH_VARRUN
#define _PATH_VARRUN 	"/var/run/"
#endif
#define _PATH_DEVNULL	"/dev/null"
#endif /* HAVE_PATHS_H */

#ifndef _ROOT_PATH
#define _ROOT_PATH	"/tmp"
#endif

#define _PATH_UPAPFILE 	 _ROOT_PATH "/ppp/pap-secrets"
#define _PATH_CHAPFILE 	 _ROOT_PATH "/ppp/chap-secrets"
#define _PATH_SRPFILE 	 _ROOT_PATH "/ppp/srp-secrets"
#define _PATH_SYSOPTIONS _ROOT_PATH "/ppp/options"
#define _PATH_IPUP	 _ROOT_PATH "/ppp/ip-up"
#define _PATH_IPDOWN	 _ROOT_PATH "/ppp/ip-down"
#define _PATH_AUTHUP	 _ROOT_PATH "/ppp/auth-up"
#define _PATH_AUTHDOWN	 _ROOT_PATH "/ppp/auth-down"
#define _PATH_TTYOPT	 _ROOT_PATH "/ppp/options."
#define _PATH_CONNERRS	 "/var/log/ppp-connect-errors"
#define _PATH_PEERFILES	 _ROOT_PATH "/ppp/peers/"
#define _PATH_RESOLV	 _ROOT_PATH "/ppp/resolv.conf"

#define _PATH_USEROPT	 ".ppprc"
#define	_PATH_PSEUDONYM	 ".ppp_pseudonym"

#ifdef INET6
#define _PATH_IPV6UP     _ROOT_PATH "/ppp/ipv6-up"
#define _PATH_IPV6DOWN   _ROOT_PATH "/ppp/ipv6-down"
#endif

#ifdef IPX_CHANGE
#define _PATH_IPXUP	 _ROOT_PATH "/ppp/ipx-up"
#define _PATH_IPXDOWN	 _ROOT_PATH "/ppp/ipx-down"
#endif /* IPX_CHANGE */

#ifdef __STDC__
#define _PATH_PPPDB	_ROOT_PATH _PATH_VARRUN "pppd.tdb"
#else /* __STDC__ */
#ifdef HAVE_PATHS_H
#define _PATH_PPPDB	"/var/run/pppd.tdb"
#else
#define _PATH_PPPDB	"/var/run/pppd.tdb"
#endif
#endif /* __STDC__ */

#ifdef PLUGIN
#define _PATH_PLUGIN	"/usr/lib/pppd"
#endif /* PLUGIN */
