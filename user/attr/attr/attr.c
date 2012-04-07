/*
 * Copyright (c) 2000-2002 Silicon Graphics, Inc.  All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <locale.h>

#include <attr/attributes.h>
#include "config.h"

#define	SETOP		1		/* do a SET operation */
#define	GETOP		2		/* do a GET operation */
#define	REMOVEOP	3		/* do a REMOVE operation */

static char *progname;

void
usage(void)
{
	fprintf(stderr, _(
"Usage: %s [-LRq] -s attrname [-V attrvalue] pathname  # set value\n"
"       %s [-LRq] -g attrname pathname                 # get value\n"
"       %s [-LRq] -r attrname pathname                 # remove attr\n"
"      -s reads a value from stdin and -g writes a value to stdout\n"),
		progname, progname, progname);
	exit(1);
}

int
main(int argc, char **argv)
{
	char *attrname, *attrvalue, *filename;
	int attrlength;
	int opflag, ch, error, follow, verbose, rootflag;

	progname = basename(argv[0]);

	setlocale(LC_CTYPE, "");
	setlocale(LC_MESSAGES, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	/*
	 * Pick up and validate the arguments.
	 */
	verbose = 1;
	follow = opflag = rootflag = 0;
	attrname = attrvalue = NULL;
	while ((ch = getopt(argc, argv, "s:V:g:r:qLR")) != EOF) {
		switch (ch) {
		case 's':
			if ((opflag != 0) && (opflag != SETOP)) {
				fprintf(stderr,
				    _("Only one of -s, -g, or -r allowed\n"));
				usage();
			}
			opflag = SETOP;
			attrname = optarg;
			break;
		case 'V':
			if ((opflag != 0) && (opflag != SETOP)) {
				fprintf(stderr,
				    _("-V only allowed with -s\n"));
				usage();
			}
			opflag = SETOP;
			attrvalue = optarg;
			break;
		case 'g':
			if (opflag) {
				fprintf(stderr,
				    _("Only one of -s, -g, or -r allowed\n"));
				usage();
			}
			opflag = GETOP;
			attrname = optarg;
			break;
		case 'r':
			if (opflag) {
				fprintf(stderr,
				    _("Only one of -s, -g, or -r allowed\n"));
				usage();
			}
			opflag = REMOVEOP;
			attrname = optarg;
			break;
		case 'L':
			follow++;
			break;
		case 'R':
			rootflag++;
			break;
		case 'q':
			verbose = 0;
			break;
		default:
			fprintf(stderr,
			    _("Unrecognized option: %c\n"), (char)ch);
			usage();
			break;
		}
	}
	if (optind != argc-1) {
		fprintf(stderr, _("A filename to operate on is required\n"));
		usage();
	}
	filename = argv[optind];

	/*
	 * Break out into option-specific processing.
	 */
	switch (opflag) {
	case SETOP:
		if (attrvalue == NULL) {
			attrvalue = malloc(ATTR_MAX_VALUELEN);
			if (attrvalue == NULL) {
				perror("malloc");
				exit(1);
			}
			attrlength =
				fread(attrvalue, 1, ATTR_MAX_VALUELEN, stdin);
		} else {
			attrlength = strlen(attrvalue);
		}
		error = attr_set(filename, attrname, attrvalue,
					   attrlength,
					   (!follow ? ATTR_DONTFOLLOW : 0) |
					   (rootflag ? ATTR_ROOT : 0));
		if (error) {
			perror("attr_set");
			fprintf(stderr, _("Could not set \"%s\" for %s\n"),
					attrname, filename);
			exit(1);
		}
		if (verbose) {
			printf(_("Attribute \"%s\" set to a %d byte value "
			       "for %s:\n"), attrname, attrlength, filename);
			fwrite(attrvalue, 1, attrlength, stdout);
			printf("\n");
		}
		break;

	case GETOP:
		attrvalue = malloc(ATTR_MAX_VALUELEN);
		if (attrvalue == NULL) {
			perror("malloc");
			exit(1);
		}
		attrlength = ATTR_MAX_VALUELEN;
		error = attr_get(filename, attrname, attrvalue,
					   &attrlength,
					   (!follow ? ATTR_DONTFOLLOW : 0) |
					   (rootflag ? ATTR_ROOT : 0));
		if (error) {
			perror("attr_get");
			fprintf(stderr, _("Could not get \"%s\" for %s\n"),
					attrname, filename);
			exit(1);
		}
		if (verbose) {
			printf(_("Attribute \"%s\" had a %d byte value "
				"for %s:\n"), attrname, attrlength, filename);
		}
		fwrite(attrvalue, 1, attrlength, stdout);
		if (verbose) {
			printf("\n");
		}
		break;

	case REMOVEOP:
		error = attr_remove(filename, attrname,
					      (!follow ? ATTR_DONTFOLLOW : 0) |
					      (rootflag ? ATTR_ROOT : 0));
		if (error) {
			perror("attr_remove");
			fprintf(stderr, _("Could not remove \"%s\" for %s\n"),
					attrname, filename);
			exit(1);
		}
		break;

	default:
		fprintf(stderr,
			_("At least one of -s, -g, or -r is required\n"));
		usage();
		break;
	}

	return(0);
}
