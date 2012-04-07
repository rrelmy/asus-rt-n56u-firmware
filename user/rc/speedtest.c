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
 * History	: 2006.05.22 	Port to WL500gP
		  2006.07.18 	Port to WL700g, and remove execution message form release version
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

extern int detect_upload_speed(double *upload_speed_p);
int qos_get_wan_rate()
{
	int count = 3, i = 0;
	char ubw_buf[128];
	double upload_speed;

	if (nvram_match("wan0_proto", "static"))
		count = 10;

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

		if (strstr(ubw_buf, "nan"))
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

	printf("\nSpeedtest init\n");	// tmp test

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

	if (!nvram_match("qos_manual_ubw","") && !nvram_match("qos_manual_ubw","0") && (atoi(nvram_safe_get("qos_manual_ubw")) > 0))
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

