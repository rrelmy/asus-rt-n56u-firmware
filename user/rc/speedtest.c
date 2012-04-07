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
/*****************************************************************************************************************
 * Filr Name	: speedtest.c
 * Description  : modify from ping.c
 * related file : qosutils.h qosutils.c
 * Auther       : Wendel Huang
 * History	: 2006.05.22 	post to WL500gP
		  2006.07.18 	Post to WL700g, and remove execution message form release version
		  2006.08.01    Debug the error in MER mode
 *****************************************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <shutils.h>
#include <rc_event.h>
#include "qosutils.h"
#include <nvram/bcmnvram.h>

#include <sys/syscall.h>
_syscall2( int, track_flag, int *, flag, ulong *, ipaddr);

enum { WAN_RATE_SUCCESS, WAN_LINK_FAIL };

#if 0

#define DETECT_FILE "/tmp/detect_ip"
#define MAX_TTL 4

static char gateway_ip[MAX_TTL][30], get_sp[MAX_TTL][10];
static int detect();

/**************************************************************************/

static int detect()
{
	FILE *fp=NULL;
	char cmd[128];
	char detect_ip[16]="18.52.86.150";
	char line[254];
	char get_ip[MAX_TTL][30];
	char *no_host="!H", *no_net="!N", *tr_str = "traceroute to";
	int j;

	printf("## detect ##\n");	// tmp test

	j = 0;
	while (j < MAX_TTL)
	{
		memset(&get_ip[j], 0, sizeof(get_ip[j]));
		memset(&gateway_ip[j], 0, sizeof(gateway_ip[j]));
		memset(&get_sp[j], 0, sizeof(get_sp[j]));
		++j;
	}
	snprintf(cmd,sizeof(cmd),"traceroute -m %d %s >%s", MAX_TTL, detect_ip, DETECT_FILE);
	remove(DETECT_FILE);
	int i;
	int ping=0;
	char *p;
	char *ms_p, *line_p, *tmp_p, tmp_buf[10]; 
	double ms_num, ms_n, ms_avg, sp, base_ms;
	nvram_set("no_internet_detect", "1");
	if (pids("tcpcheck"))
		system("killall -SIGTERM tcpcheck");
	if (pids("traceroute"))
		system("killall traceroute");

	system(cmd);

	i = 0;
	if ((fp = fopen(DETECT_FILE, "r")) != NULL) 
	{
		while (i < MAX_TTL)
		{
			/* get ip */
			if ( fgets(line, sizeof(line), fp) != NULL ) 
			{
				if (strstr(line, no_host) || strstr(line, no_net))	// add
				{
					printf("detect: no host or no network\n");	// tmp test
					fclose(fp);
					nvram_set("no_internet_detect", "0");
					return -1;
				}

				if (strstr(line, tr_str))
					continue;

				p=index(line,'(');
				if (!p)
					continue;

				j=1;
				while ((*p)!=')')
				{
					++p;
					get_ip[i][j-1]=*p;
					++j;
				}
				get_ip[i][j-2]='\0';
				
			}

			/* get speed */
			ms_num = 0.0;
			ms_n = 0;
			line_p = line;
			while ((ms_p = strstr(line_p, "ms")) != NULL)
			{
				++ms_n;
				tmp_p = ms_p-2;
				line_p = ms_p + 1;
				j = 0;
				memset(tmp_buf, 0, sizeof(tmp_buf));

				for (; *tmp_p!= ' '; --tmp_p, ++j)
				{
					tmp_buf[9-j] = *tmp_p;
				}
				j=0;
				while ((tmp_buf[j]=='\0') && (j < 10))
				{
					tmp_buf[j] = '0';
					++j;
				}

				ms_num += atof(tmp_buf);
				//printf("get ms from [%s]: %f, (%s)\n", get_ip[i], ms_num, tmp_buf);	// tmp test
			}
			if (ms_num > 0.0)
			{
				memset(tmp_buf, 0, sizeof(tmp_buf));
				ms_avg = ms_num / ms_n;

				/* judge codes */
				if (i==1)
				{
					base_ms = ms_avg;
					printf("base ms is %f\n", base_ms);	// tmp test
				}
				else if (i > 1)
				{
					if (ms_avg > (3*base_ms))
					{
						printf("!(over 3*base) %f\n", ms_avg);	// tmp test
						ms_avg = base_ms*(1.30);
						printf("judge that as %f\n", ms_avg);	// tmp test
					}
					else if (ms_avg > (2*base_ms))
					{
						printf("!(over the 2*base) %f\n", ms_avg);	// tmp test
						ms_avg = base_ms*(1.20);
						printf("judge that as %f\n", ms_avg);		// tmp test
					}
				}

//				sp = (((38.00/1024.00)/(ms_avg/1000000)))/2;	// J++
				sp = (((38.00/1024.00)/(ms_avg/1000000)));
				sprintf(get_sp[i], "%.2f", sp);
				printf("get ms avg : %f(%f), sp=%s\n", ms_avg, ms_num, get_sp[i]);	// tmp test
			}
			++i;
		}

		i=0;
		while (i<MAX_TTL)
		{
			sprintf(gateway_ip[i],"%s",get_ip[i]);
			++i;
		}	
	}
	else
	{
		nvram_set("no_internet_detect", "0");
		return 0;
	}

	fclose(fp);
	printf("end detect\n");	// tmp test
	nvram_set("no_internet_detect", "0");
	return 1;
}

/**************************************************************************
 * Function Name: qos_get_wan_rate
 * Description  : get the ADSL line rate
 * Parameters   : None
 * Returns      : Is successful when get the ADSL data rate? 
			0 : Success
			1 : Link fail
 **************************************************************************/
int qos_get_wan_rate()
{
	char us[256], ds[256] ;
	char * gw_ip=NULL;
	struct timeval deltaval;
	double delta_us = 0;
	int count = 3;
	char *ip = nvram_safe_get("wan_dns_t");
	int max_request_time;
	double sp, sp_avg, sp_valid, tmp_add;
	char ubw_buf[128];
	int i = 0;

	printf("\nQos get wan rate\n");	// tmp test

	while (!check_wan_link(0))
	{
		printf("check wan link fail\n");	// tmp test

		if (nvram_match("wan0_proto", "static"))
			count = 4;

		if (i++ < count)
			sleep(1);
		else
			return WAN_LINK_FAIL;
	}

	gw_ip = nvram_safe_get("lan_ipaddr");
	//printf("gw ip is %s\n", gw_ip);	// tmp test

	if ((strchr(ip, ' ')) ||(!strcmp(ip, "")))
	{
		struct dns_lists *dns_list = get_dns_list(1);

//		printf("fall 1\n");	// tmp test
		for (i=0 ; i<dns_list->num_servers ; i++) {
			ip = dns_list->dns_server[i];
			if (    (!strchr(ip, ' ')) 
					&& (strcmp(ip, ""))
					&& (strcmp(ip, gw_ip))) //WL600g auto assign LAN IP as DNS Server 
				break;
		}
		free(dns_list);
	}

	//if ( (strchr(ip, ' ')) || (!strcmp(ip, "")) || (!strcmp(ip, gw_ip)) ) {
	if ( (!strcmp(ip, "")) || (!strcmp(ip, gw_ip)) ) {
/*
		if (strchr(ip, ' '))
			printf("wan get rate fail 1\n");	// tmp test
		if (!strcmp(ip, ""))
			printf("wan get rate fail 2\n");	// tmp test
		if (!strcmp(ip, gw_ip))
			printf("wan get rate fail 3\n");	// tmp test
*/
		nvram_set("qos_ubw_status", "fail");
		nvram_set("qos_ubw_reason", "DNS server fail");
		nvram_set("qos_ubw", "0");
		return WAN_LINK_FAIL;
	}
	else 
	{
		if (detect() == 1)
		{
			tmp_add = 0.0;
			sp_valid = 0.0;
			for (i=1; i<6; ++i)	/* discard first gateway */
			{
				if (strlen(gateway_ip[i]) > 0)
				{
					//printf("test [%s][%s]\n", gateway_ip[i], get_sp[i]);	// tmp test
					if ((sp=atof(get_sp[i])) > 0.0)
					{
						tmp_add += sp;
						++sp_valid;
						//printf("tmp_add is %f, valid is %f\n", sp, sp_valid);	// tmp test
					}
				}
			}
			sp_avg = tmp_add/sp_valid;
			memset(ubw_buf, 0, sizeof(ubw_buf));
			sprintf(ubw_buf, "%.2f", sp_avg);

			if (!strcmp(ubw_buf, "nan") || !strcmp(ubw_buf, "-nan"))
			{
				nvram_set("qos_ubw_status", "fail");
				nvram_set("qos_ubw_reason", "detect fail");
				nvram_set("qos_ubw", "0");

				return WAN_LINK_FAIL;
			}

			nvram_set("qos_ubw_status", "success");
			nvram_set("qos_ubw", ubw_buf);
			nvram_set("qos_ubw_reason", "");

			printf("Sp avg is %s (%f)(%f)\n", ubw_buf, sp_valid, tmp_add);	// tmp test

			return WAN_RATE_SUCCESS;
		} 
		else
		{
			nvram_set("qos_ubw_status", "fail");
			nvram_set("qos_ubw_reason", "detect fail");
			nvram_set("qos_ubw", "0");
			return WAN_LINK_FAIL;
		}
	}
}
#else
extern int detect_upload_speed(double *upload_speed_p);
int qos_get_wan_rate()
{
	int count = 3, i = 0;
	char ubw_buf[128];
	double upload_speed;

	printf("\nQos get wan rate\n");	// tmp test

	while (!check_wan_link(0))
	{
		printf("check wan link fail\n");	// tmp test

		if (++i < count)
			sleep(1);
		else
			return WAN_LINK_FAIL;
	}

	sleep(1);

	if (detect_upload_speed(&upload_speed) == 1)
	{
		memset(ubw_buf, 0, sizeof(ubw_buf));
		sprintf(ubw_buf, "%.2f", upload_speed);

		if (!strcmp(ubw_buf, "nan") || !strcmp(ubw_buf, "-nan"))
		{
			nvram_set("qos_ubw_status", "fail");
			nvram_set("qos_ubw_reason", "detect fail");
			nvram_set("qos_ubw", "0");

			return WAN_LINK_FAIL;
		}

		nvram_set("qos_ubw_status", "success");
		nvram_set("qos_ubw_reason", "");
		nvram_set("qos_ubw", ubw_buf);

		return WAN_RATE_SUCCESS;
	} 
	else
	{
		nvram_set("qos_ubw_status", "fail");
		nvram_set("qos_ubw_reason", "detect fail");
		nvram_set("qos_ubw", "0");

		return WAN_LINK_FAIL;
	}
}
#endif

#define isdigit(c) (c >= '0' && c <= '9') 
static long atoid(const char *s)
{
	 int i=0,j;
	 long k=0;
	 for (j=1; j<5; j++) {
		 i=0;
		 while (isdigit(*s)) {
			 i = i*10 + *(s++) - '0';
		}
		 k = k*256 +i;
		 if (j == 4)
			 return k;
		 s++;
	}
	 return k;
}

int flag = 0;
ulong ipaddr = 0;

void
track_set(char *c_track)
{
	int k_track = atoi(c_track);
	ipaddr = atoid(nvram_safe_get("wan_ipaddr_t"));

	printf("track set %d\n", k_track); // tmp test
	if (!track_flag(&k_track, &ipaddr))
	{
		printf("track ok:%d\n", k_track);
		event_code = k_track;
	}
	else
		printf("track fail\n");

}

int Speedtest_Init(void) {
	char net_name[32];
	FILE *fp=NULL;
	char pbuf[128];	// tmp test

//	nvram_set("qos_userspec_app", "0");
//	nvram_set("qos_global_enable", "0");
//	nvram_set("qos_userdef_enable", "0");

	printf("\nSpeedtest init\n");	// tmp test

//	if ( nvram_invmatch("qos_rulenum_x", "0"))
//		nvram_set("qos_userspec_app", "1");
	
	//add by Angela 2008.05
//	if (nvram_match("qos_tos_prio", "1")||nvram_match("qos_pshack_prio", "1")
//	  || nvram_match("qos_service_enable", "1")|| nvram_match("qos_shortpkt_prio", "1"))
//		nvram_set("qos_global_enable", "1");
	
//	if (nvram_invmatch("qos_rulenum_x", "0") || nvram_match("qos_dfragment_enable", "1"))
//		nvram_set("qos_userdef_enable", "1");

	/* Get interface name */
	if (nvram_match("wan0_proto", "pppoe") || nvram_match("wan0_proto","pptp") || nvram_match("wan0_proto","l2tp"))
		strcpy (net_name, nvram_safe_get("wan0_pppoe_ifname"));
	else	
		strcpy (net_name, nvram_safe_get("wan0_ifname"));

	/* Reset all qdisc first */
	doSystem("tc qdisc del dev %s root htb", net_name);
	system("tc qdisc del dev br0 root htb");

	/* Clean iptables*/
	/*system("iptables -F -t mangle");*/
	if ((fp=fopen("/tmp/mangle_rules", "w"))==NULL) 
	{
		nvram_set("qos_file"," /tmp/mangle_rules file does not exist.");
		if ((fp = fopen("/tmp/mangle_rules", "w+")) == NULL)
			return ;
	}
	fprintf(fp, "*mangle\n");
	fprintf(fp, "-F\n");
	fprintf(fp, "COMMIT\n\n");
	fclose(fp);
	system("iptables-restore /tmp/mangle_rules");

	if (nvram_invmatch("qos_manual_ubw","") && nvram_invmatch("qos_manual_ubw","0") && (atoi(nvram_safe_get("qos_manual_ubw")) > 0))
		nvram_set("qos_ubw_real", nvram_safe_get("qos_manual_ubw"));
	else 
		nvram_set("qos_ubw_real", nvram_safe_get("qos_ubw"));

	int qos_userspec_app_en = 0;
	int rulenum = atoi(nvram_safe_get("qos_rulenum_x")), idx_class = 0;
	/* Add class for User specify, 10:20(high), 10:40(middle), 10:60(low)*/
	if (rulenum) {
		for (idx_class=0; idx_class < rulenum; idx_class++)
		{
			if (atoi(Ch_conv("qos_prio_x", idx_class)) == 1)
			{
				qos_userspec_app_en = 1;
				break;
			}
			else if (atoi(Ch_conv("qos_prio_x", idx_class)) == 6)
			{
				qos_userspec_app_en = 1;
				break;
			}
		}
	}

//2008.10 magic{
//	if (nvram_match("qos_global_enable", "1") || nvram_match("qos_userdef_enable", "1"))
	if (	(nvram_match("qos_tos_prio", "1") ||
		 nvram_match("qos_pshack_prio", "1") ||
		 nvram_match("qos_service_enable", "1") ||
		 nvram_match("qos_shortpkt_prio", "1") ||
		 (rulenum && qos_userspec_app_en)) &&
		(atoi(nvram_safe_get("qos_ubw_real")) > 0)	)
	{
		nvram_set("qos_enable", "1");
		track_set("1");
		flag = 1;

		ipaddr = atoid(nvram_safe_get("wan_ipaddr_t"));

		start_qos();
	}
	else
	{
		nvram_set("qos_enable", "0");
		track_set("0");
		flag = 0;
	}

//2008.10 magic}
	if (!track_flag(&flag, &ipaddr))
		nvram_set("qos_sys", "right set");
	else 
		nvram_set("qos_sys", "error set");
	return 0;
}

