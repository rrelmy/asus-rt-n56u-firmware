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
 * $Id: options.h,v 1.1.1.1 2007/02/15 12:14:18 jiahao Exp $
 *
 * Copyright (C) 1996 Lars Fenneberg
 *
 * See the file COPYRIGHT for the respective terms and conditions. 
 * If the file is missing contact me at lf@elemental.net 
 * and I'll send you a copy.
 *
 */

#define OPTION_LEN	64

/* ids for different option types */
#define OT_STR		(1<<0)	  /* string */
#define OT_INT		(1<<1)	  /* integer */
#define OT_SRV		(1<<2)	  /* server list */
#define OT_AUO		(1<<3)    /* authentication order */

#define OT_ANY		((unsigned int)~0) /* used internally */

/* status types */
#define ST_UNDEF	(1<<0)	  /* option is undefined */

typedef struct _option {
	char name[OPTION_LEN];	  /* name of the option */
	int type, status;	  /* type and status    */
	void *val;		  /* pointer to option value */
} OPTION;

static SERVER acctserver = {0};
static SERVER authserver = {0};

int default_tries = 4;
int default_timeout = 60;

static OPTION config_options[] = {
/* internally used options */
{"config_file",		OT_STR, ST_UNDEF, NULL},
/* General options */
{"auth_order",	 	OT_AUO, ST_UNDEF, NULL},
{"login_tries",	 	OT_INT, ST_UNDEF, &default_tries},
{"login_timeout",	OT_INT, ST_UNDEF, &default_timeout},
{"nologin",		OT_STR, ST_UNDEF, "/etc/nologin"},
{"issue",		OT_STR, ST_UNDEF, "/etc/radiusclient/issue"},
/* RADIUS specific options */
{"authserver",		OT_SRV, ST_UNDEF, &authserver},
{"acctserver",		OT_SRV, ST_UNDEF, &acctserver},
{"servers",		OT_STR, ST_UNDEF, NULL},
{"dictionary",		OT_STR, ST_UNDEF, NULL},
{"login_radius",	OT_STR, ST_UNDEF, "/usr/sbin/login.radius"},
{"seqfile",		OT_STR, ST_UNDEF, NULL},
{"mapfile",		OT_STR, ST_UNDEF, NULL},
{"default_realm",	OT_STR, ST_UNDEF, NULL},
{"radius_timeout",	OT_INT, ST_UNDEF, NULL},
{"radius_retries",	OT_INT,	ST_UNDEF, NULL},
{"nas_identifier",      OT_STR, ST_UNDEF, ""},
/* local options */
{"login_local",		OT_STR, ST_UNDEF, NULL},
};

static int num_options = ((sizeof(config_options))/(sizeof(config_options[0])));
