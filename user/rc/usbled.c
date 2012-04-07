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
 *
 * Copyright 2010, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <nvram/bcmnvram.h>
#include <rtxxxx.h>

#define NORMAL_PERIOD		1		/* second */
#define URGENT_PERIOD		100 * 1000	/* microsecond */	

struct itimerval itv;

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

extern int linkstatus_usb;

void nousbled(int sig)
{
	alarmtimer(0, 0);
	linkstatus_usb = -1;
	nvram_set("no_usb_led", "0");
	remove("/var/run/usbled.pid");
	exit(0);
}

int count = 0;

void usbled(int sig)
{
	char *usb_path1 = nvram_safe_get("usb_path1");
	char *usb_path2 = nvram_safe_get("usb_path2");

	if (strcmp(usb_path1, "storage") && strcmp(usb_path2, "storage"))
	{
		nousbled(sig);
	}
	else
	{
		count = (++count % 20);

		/* 0123456710 */
		/* 1010101010 */
		if (((count % 2) == 0) && (count > 8))
			LED_CONTROL(LED_USB, LED_ON);
		else
			LED_CONTROL(LED_USB, LED_OFF);

		alarmtimer(0, URGENT_PERIOD);
	}
}

int 
usbled_main(int argc, char *argv[])
{
	FILE *fp;

	/* write pid */
	if ((fp = fopen("/var/run/usbled.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	nvram_set("no_usb_led", "1");

	/* set the signal handler */
	signal(SIGALRM, usbled);
	signal(SIGTERM, nousbled);

	alarmtimer(0, URGENT_PERIOD);

	/* Most of time it goes to sleep */
	while (1)
	{
		pause();
	}

	return 0;
}
