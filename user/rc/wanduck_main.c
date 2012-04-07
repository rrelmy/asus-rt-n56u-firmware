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
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

#include <nvram/bcmnvram.h>
#include <shutils.h>
#include <rc.h>
#include <stdarg.h>

static int run_wanduck() {
	char *argv[] = {"/usr/sbin/wanduck", NULL};
printf("--- get signal to start wanduck in wanduckmain. ---\n");	// tmp test
	return _eval(argv, NULL, 0, NULL);
}

static int remove_wanduck() {
	char *argv[] = {"killall", "wanduck", NULL};
printf("--- get signal to stop wanduck in wanduckmain. ---\n");
	return _eval(argv, NULL, 0, NULL);
}

static int child_dead() {
printf("--- wanduck was dead! ---\n");
}

static int safe_leave() {
	char *argv[] = {"rm", "-f", "/var/run/wanduckmain.pid", NULL};
	int ret;
printf("--- get signal to leave wanduckmain. ---\n");
	ret = _eval(argv, NULL, 0, NULL);
	exit(0);
}

int wanduck_main(void) {
   printf("******in wanduck_main \n");
	FILE *fp;	
	int ret;
	sigset_t sigset;
	
	fp = fopen("/var/run/wanduckmain.pid", "w");
	if (fp == NULL)
		exit(0);
//printf("******in wanduck_main2222 \n");
	
	fprintf(fp, "%d", getpid());
	fclose(fp);
	
	signal(SIGUSR1, run_wanduck);
	signal(SIGUSR2, remove_wanduck);
	signal(SIGCHLD, child_dead);
	signal(SIGTERM, safe_leave);
	sigemptyset(&sigset);
	//printf("******in wanduck_main3333 \n");
	while (1) {
		sigsuspend(&sigset);
	}
}
