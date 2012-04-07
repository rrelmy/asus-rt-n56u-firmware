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
/***********************************************************************
*
* pty.c
*
* Code for dealing with pseudo-tty's for running pppd.
*
* Copyright (C) 2002 Roaring Penguin Software Inc.
*
* This software may be distributed under the terms of the GNU General
* Public License, Version 2, or (at your option) any later version.
*
* LIC: GPL
*
***********************************************************************/

static char const RCSID[] =
"$Id: pty.c,v 1.2 2002/09/30 19:45:00 dskoll Exp $";

#include "../l2tp.h"
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#ifndef N_HDLC
#include <linux/termios.h>
#endif

/**********************************************************************
* %FUNCTION: pty_get
* %ARGUMENTS:
*  mfp -- pointer to master file descriptor
*  sfp -- pointer to slave file descriptor
* %RETURNS:
*  0 on success, -1 on failure
* %DESCRIPTION:
*  Opens a PTY and sets line discipline to N_HDLC for ppp.
*  Taken almost verbatim from Linux pppd code.
***********************************************************************/
int
pty_get(int *mfp, int *sfp)
{
    char pty_name[24];
    struct termios tios;
    int mfd, sfd;
    int disc = N_HDLC;

    mfd = -1;
    sfd = -1;

    mfd = open("/dev/ptmx", O_RDWR);
    if (mfd >= 0) {
	int ptn;
	if (ioctl(mfd, TIOCGPTN, &ptn) >= 0) {
	    snprintf(pty_name, sizeof(pty_name), "/dev/pts/%d", ptn);
	    ptn = 0;
	    if (ioctl(mfd, TIOCSPTLCK, &ptn) < 0) {
		/* warn("Couldn't unlock pty slave %s: %m", pty_name); */
	    }
	    if ((sfd = open(pty_name, O_RDWR | O_NOCTTY)) < 0) {
		/* warn("Couldn't open pty slave %s: %m", pty_name); */
	    }
	}
    }

    if (sfd < 0 || mfd < 0) {
	if (sfd >= 0) close(sfd);
	if (mfd >= 0) close(mfd);
	return -1;
    }

    *mfp = mfd;
    *sfp = sfd;
    if (tcgetattr(sfd, &tios) == 0) {
	tios.c_cflag &= ~(CSIZE | CSTOPB | PARENB);
	tios.c_cflag |= CS8 | CREAD | CLOCAL;
	tios.c_iflag  = IGNPAR;
	tios.c_oflag  = 0;
	tios.c_lflag  = 0;
	tcsetattr(sfd, TCSAFLUSH, &tios);
    }
    if (ioctl(sfd, TIOCSETD, &disc) < 0) {
	l2tp_set_errmsg("Unable to set line discipline to N_HDLC");
	close(mfd);
	close(sfd);
	return -1;
    }
    disc = N_HDLC;
    if (ioctl(mfd, TIOCSETD, &disc) < 0) {
	l2tp_set_errmsg("Unable to set line discipline to N_HDLC");
	close(mfd);
	close(sfd);
	return -1;
    }
    return 0;
}

