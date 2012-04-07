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
#ifndef _asus_ddns_h_
#define _asus_ddns_h_

// global variables (asus_ddns.c)
extern int	g_asus_ddns_mode;

// global variables (ez-ipupdate.c)
extern char *program_name;
extern char *cache_file;
extern char *config_file;
extern char *server;
extern char *port;
extern char user[256];
extern char auth[512];
extern char user_name[128];
extern char password[128];
extern char *address;
extern char *request;
extern int wildcard;
extern char *mx;
extern char *url;
extern char *host;
extern char *cloak_title;
extern char *interface;
extern int ntrys;
extern int update_period;
extern int resolv_period;
extern struct timeval timeout;
extern int max_interval;
extern int service_set;
extern char *post_update_cmd;
extern char *post_update_cmd_arg;
extern int connection_type;
extern char *notify_email;
extern char *pid_file;
extern int options;
extern volatile int client_sockfd;


// function decl. (asus_ddns.c)
extern int asus_reg_domain (void);
extern int asus_private(void);
extern int asus_update_entry(void);

// function decl. (ez-ipupdate.c)
extern int do_connect(int *sock, char *host, char *port);

#endif
