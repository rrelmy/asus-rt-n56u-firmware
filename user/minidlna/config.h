/* MiniDLNA Project
 * http://sourceforge.net/projects/minidlna/
 * (c) 2008-2009 Justin Maggard
 * generated by ./genconfig.sh on Mon Jun 27 10:44:47 CST 2011 */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define OS_NAME			"Linux"
#define OS_VERSION		"Linux/2.6.34.9-69.fc13.i686.PAE"
#define OS_URL			"http://www.kernel.org/"

/* full path of the file database */
//#define DEFAULT_DB_PATH		"/tmp/minidlna"
/* ASUS EXT */
#define DEFAULT_DB_PATH		"/tmp/harddisk/part0/.dms"

/* full path of the log directory */
//#define DEFAULT_LOG_PATH	"/tmp/minidlna"
/* ASUS EXT */
#define DEFAULT_LOG_PATH	"/tmp/harddisk/part0/.dms"

/* Comment the following line to use home made daemonize() func instead
 * of BSD daemon() */
#define USE_DAEMON

/* Enable if the system inotify.h exists.  Otherwise our own inotify.h will be used. */
/* ASUS EXT */
//#define HAVE_INOTIFY_H

/* Enable if the system iconv.h exists.  ID3 tag reading in various character sets will not work properly otherwise. */
#define HAVE_ICONV_H

/* Enable if the system libintl.h exists for NLS support. */
/* ASUS EXT */
//#define ENABLE_NLS

/* Enable NETGEAR-specific tweaks. */
/*#define NETGEAR*/
/* Enable ReadyNAS-specific tweaks. */
/*#define READYNAS*/
/* Compile in TiVo support. */
/*#define TIVO_SUPPORT*/
/* Enable PnPX support. */
#define PNPX 0

#endif
