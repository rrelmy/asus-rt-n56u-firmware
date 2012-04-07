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
#ifndef _SH_H
#define _SH_H

#pragma pack(1)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>
#include <syslog.h>
#include <sys/param.h>	// for the variable, NOFILE
#include <stdarg.h>	// 2008.02 James.

#define socklen_t int
#include <net/if.h>
#include <shutils.h>
#include <ethtool-util.h>
#include <nvram/bcmnvram.h>

#include "nvram/typedefs.h"
#include "rc.h"

#ifdef RTCONFIG_USB_MODEM
#define MAX_WAIT_TIME_OF_MODEM 60
#define MAX_WAIT_COUNT MAX_WAIT_TIME_OF_MODEM/SCAN_INTERVAL
#define MODEM_IDLE -1
#define MODEM_READY 0
int link_modem = 0;
int modem_running = 0;
#define MAX_WAIT_OF_MODEM 10
#endif

#define SCAN_INTERVAL 1

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

#ifndef SIOCETHTOOL
#define SIOCETHTOOL 0x8946
#endif

#ifndef SIOCGIFADDR
#define SIOCGIFADDR 0x8915
#endif 

#ifndef SIOCGETCPHYRD
#define SIOCGETCPHYRD 0x89FE
#endif

int fer = 0;
int sw_mode = 0;

#define MAXLINE				2048
#define PATHLEN				256
#define STRLEN				256
//#define HOSTLEN			64
#define DFL_HTTP_SERV_PORT	"18017"
#define DFL_DNS_SERV_PORT	"18018"
#define MAX_USER			100

#define GET			'g'
#define POST		'p'

#define T_HTTP	'H'
#define T_DNS		'D'

#define CHK_PPP		1
#define CHK_DHCPD	2

#define DISCONN	0
#define CONNED	1
#define C2D			3
#define D2C			4

#define CASE_DISWAN        1
#define CASE_PPPFAIL       2
#define CASE_DHCPFAIL      3
#define CASE_MISROUTE      4
#define CASE_OTHERS        5
#define CASE_THESAMESUBNET 6	// 2008.03 James.
#define CASE_PPPNOIP       7

#define CASE_FIRST_REPEATER	100

#define OK_STR	"Content-Type: text/html\r\n"

typedef struct Flags_Pack{
        uint16_t reply_1:4,     // bit:0-3
        non_auth_ok:1,          // bit:4
        answer_auth:1,          // bit:5
        reserved:1,             // bit:6
        recur_avail:1,          // bit:7
        recur_desired:1,        // bit:8
        truncated:1,            // bit:9
        authori:1,              // bit:10
        opcode:4,               // bit:11-14
        response:1;             // bit:15
} flag_pack;

typedef struct DNS_HEADER{
        uint16_t tid;
        union {
                uint16_t flag_num;
                flag_pack flags;
        } flag_set;
        uint16_t questions;
        uint16_t answer_rrs;
        uint16_t auth_rrs;
	uint16_t additional_rss;
} dns_header;

typedef struct QUERIES{
	char name[PATHLEN];
	uint16_t type;
	uint16_t ip_class;
} dns_queries;

typedef struct ANSWER{
	uint16_t name;
	uint16_t type;
	uint16_t ip_class;
	uint32_t ttl;	// time to live
	uint16_t data_len;
	uint32_t addr;
} dns_answer;

typedef struct DNS_REQUEST{
	dns_header header;
	dns_queries queries;
} dns_query_packet;

typedef struct DNS_RESPONSE{
	dns_header header;
	char *q_name;
	uint16_t q_type;
	uint16_t q_class;
	dns_answer answers;
} dns_response_packet;

typedef struct REQCLIENT{
	int sfd;
	char type;
} clients;

// var
int http_sock, dns_sock, maxfd;
clients client[MAX_USER];
fd_set  rset, allset;
int fd_i;
int cur_sockfd, main_pid, connfd, rule_setup, nat_enable;
char dst_url[256];
int disconn_case;
int err_state;
//bool is_wsc_configed;
typedef unsigned char   bool;
bool isFirstUse;
#define NAT_FILE "/tmp/nat_rules"
#define REDIRECT_FILE "/tmp/redirect_rules"
char *add_command[] = {"iptables-restore", REDIRECT_FILE, NULL};
char *del_command[] = {"iptables-restore", NAT_FILE, NULL};

int Dr_Surf_case = 0;	// 2008.02 James.

int wan_ready = 0;
int wan_unit = 0;
char wan0_ifname[16];
char wan1_ifname[16];
char wan0_proto[16];
char wan1_proto[16];
char wan_gateway_t[16];
char wan_subnet_t[11]; // 2010.09 James.
char lan_ipaddr_t[16];
char lan_subnet_t[11]; // 2010.09 James.

#define csprintf(fmt, args...) do{\
	FILE *cp = fopen("/dev/console", "w");\
	if(cp) {\
		fprintf(cp, fmt, ## args);\
		fclose(cp);\
	}\
}while(0)

// func
void change_redirect_rules(int num, int isup);
void run_http_serv(int);
void run_dns_serv(int);
int readline(int, char*, int);
void handle_http_req(int, char*);
void handle_dns_req(int, char*, int n, struct sockaddr*, int);
void send_page(int, char*, char *);
int if_wan_phyconnected();
// 2007.11 James {
void build_rule_file();
int build_socket(char *http_port, char *dns_port, int *hd, int *dd);
void record_conn_status();
void logmessage(char *logheader, char *fmt, ...);
void get_related_nvram();
void get_related_nvram2();
// 2007.11 James }
#endif
