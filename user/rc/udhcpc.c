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
 * udhcpc scripts
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: udhcpc.c,v 1.1.1.1 2007/02/15 12:14:33 jiahao Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <nvram/bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>

char udhcpstate[8];

static int
expires(char *wan_ifname, unsigned int in)
{
	time_t now;
	FILE *fp;
	char tmp[100];
	int unit;

	if ((unit = wan_ifunit(wan_ifname)) < 0)
		return -1;
	
	time(&now);
	snprintf(tmp, sizeof(tmp), "/tmp/udhcpc%d.expires", unit); 
	if (!(fp = fopen(tmp, "w"))) {
		perror(tmp);
		return errno;
	}
	fprintf(fp, "%d", (unsigned int) now + in);
	fclose(fp);
	return 0;
}	

/* 
 * deconfig: This argument is used when udhcpc starts, and when a
 * leases is lost. The script should put the interface in an up, but
 * deconfigured state.
*/
static int
deconfig(void)
{
	char *wan_ifname = safe_getenv("interface");

	if (nvram_match("wan0_proto", "l2tp") || nvram_match("wan0_proto", "pptp"))
	{
		/* fix hang-up issue */
		logmessage("dhcp client", "skipping resetting IP address to 0.0.0.0");
	} else
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
	expires(wan_ifname, 0);

	wan_down(wan_ifname);

	logmessage("dhcp client", "%s: lease is lost", udhcpstate);
	wanmessage("lost IP from server");

	return 0;
}

/*
 * bound: This argument is used when udhcpc moves from an unbound, to
 * a bound state. All of the paramaters are set in enviromental
 * variables, The script should configure the interface, and set any
 * other relavent parameters (default gateway, dns server, etc).
*/
#if 0
static int
bound(void)
{
	char *wan_ifname = safe_getenv("interface");
	char *value;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;
	char *value_ip;
	char *value_subnet;
	char *value_gateway;
	char lan_ipaddr_t[16];
	char lan_netmask_t[16];
	unsigned int lan_subnet = 0;
	unsigned int wan_subnet = 0;

	if ((unit = wan_ifunit(wan_ifname)) < 0) 
		strcpy(prefix, "wanx_");
	else
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	value_ip = getenv("ip");
	value_subnet = getenv("subnet");
	value_gateway = getenv("router");
	
	memset(lan_ipaddr_t, 0, 16);
	memset(lan_netmask_t, 0, 16);
	strcpy(lan_ipaddr_t, nvram_safe_get("lan_ipaddr_t"));
	strcpy(lan_netmask_t, nvram_safe_get("lan_netmask_t"));
	lan_subnet = inet_network(lan_ipaddr_t)&inet_network(lan_netmask_t);
	printf("[WAN udhcpc] lan subnet: %x\n", lan_subnet);

	if (value_ip && value_subnet)
	{
		wan_subnet = inet_network(trim_r(value_ip))&inet_network(trim_r(value_subnet));
		nvram_set("wan_ipaddr_tmp", trim_r(value_ip));
		nvram_set("wan_netmask_tmp", trim_r(value_subnet));
		printf("[WAN udhcpc] wan subnet: %x\n", wan_subnet);
	}

	if (value_gateway)
		nvram_set("wan_gateway_tmp", trim_r(value_gateway));

	if (lan_subnet == wan_subnet)
	{
		printf("[WAN udhcpc] LAN subnet and WAN subnet conflict!\n", wan_subnet);
		return;
	}

	nvram_set(strcat_r(prefix, "ipaddr", tmp), trim_r(value_ip));
	nvram_set(strcat_r(prefix, "netmask", tmp), trim_r(value_subnet));
	if (value_gateway)
		 nvram_set(strcat_r(prefix, "gateway", tmp), trim_r(value_gateway));
//	if ((value = getenv("ip")))
//		nvram_set(strcat_r(prefix, "ipaddr", tmp), trim_r(value));
//	if ((value = getenv("subnet")))
//		nvram_set(strcat_r(prefix, "netmask", tmp), trim_r(value));
//	if ((value = getenv("router")))
//		nvram_set(strcat_r(prefix, "gateway", tmp), trim_r(value));
	if ((value = getenv("dns")))
		nvram_set(strcat_r(prefix, "dns", tmp), trim_r(value));
	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins", tmp), trim_r(value));

	nvram_set(strcat_r(prefix, "routes", tmp), getenv("routes"));
	nvram_set(strcat_r(prefix, "msroutes", tmp), getenv("msroutes"));
#if 0
	if ((value = getenv("hostname")))
		sethostname(trim_r(value), strlen(value) + 1);
#endif
	if ((value = getenv("domain")))
		nvram_set(strcat_r(prefix, "domain", tmp), trim_r(value));
	if ((value = getenv("lease"))) {
		nvram_set(strcat_r(prefix, "lease", tmp), trim_r(value));
		expires(wan_ifname, atoi(value));
	}

	ifconfig(wan_ifname, IFUP,
		 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
		 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));

	wan_up(wan_ifname);

	logmessage("dhcp client", "%s IP : %s from %s", 
		udhcpstate, 
		nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
		nvram_safe_get(strcat_r(prefix, "gateway", tmp)));

	wanmessage("");
	dprintf("done\n");
	return 0;
}
#else
static int
bound(void)	// udhcpc bound here, also call wanup
{
	char *wan_ifname = safe_getenv("interface");
	char *value;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;

	if ((unit = wan_ifunit(wan_ifname)) < 0) 
		strcpy(prefix, "wanx_");
	else
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	
	if ((value = getenv("ip")))
		nvram_set(strcat_r(prefix, "ipaddr", tmp), trim_r(value));
	if ((value = getenv("subnet")))
		nvram_set(strcat_r(prefix, "netmask", tmp), trim_r(value));
        if ((value = getenv("router")))
		nvram_set(strcat_r(prefix, "gateway", tmp), trim_r(value));
	if ((value = getenv("dns")))
		nvram_set(strcat_r(prefix, "dns", tmp), trim_r(value));
	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins", tmp), trim_r(value));

	nvram_set(strcat_r(prefix, "routes", tmp), getenv("routes"));
	nvram_set(strcat_r(prefix, "msroutes", tmp), getenv("msroutes"));
#if 0
	if ((value = getenv("hostname")))
		sethostname(trim_r(value), strlen(value) + 1);
#endif
	if ((value = getenv("domain")))
		nvram_set(strcat_r(prefix, "domain", tmp), trim_r(value));
	if ((value = getenv("lease"))) {
		nvram_set(strcat_r(prefix, "lease", tmp), trim_r(value));
		expires(wan_ifname, atoi(value));
	}

	ifconfig(wan_ifname, IFUP,
		 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
		 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));

	nvram_set("dhcp_renew", "0");	// for detectWAN

	wan_up(wan_ifname);

	logmessage("dhcp client", "%s IP : %s from %s", 
		udhcpstate, 
		nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
		nvram_safe_get(strcat_r(prefix, "gateway", tmp)));

	wanmessage("");
	dprintf("done\n");
	return 0;
}
#endif

/*
 * renew: This argument is used when a DHCP lease is renewed. All of
 * the paramaters are set in enviromental variables. This argument is
 * used when the interface is already configured, so the IP address,
 * will not change, however, the other DHCP paramaters, such as the
 * default gateway, subnet mask, and dns server may change.
 */
static int
renew(void)
{
	char *wan_ifname = safe_getenv("interface");
	char *value;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;

	if ((unit = wan_ifunit(wan_ifname)) < 0) 
		strcpy(prefix, "wanx_");
	else
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	if (!(value = getenv("subnet")) || nvram_invmatch(strcat_r(prefix, "netmask", tmp), trim_r(value)))
		return bound();
	if (!(value = getenv("router")) || nvram_invmatch(strcat_r(prefix, "gateway", tmp), trim_r(value)))
		return bound();

	if ((value = getenv("dns")) && nvram_invmatch(strcat_r(prefix, "dns", tmp), trim_r(value))) {
		nvram_set(strcat_r(prefix, "dns", tmp), trim_r(value));
		update_resolvconf();
	}

	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins", tmp), trim_r(value));
#if 0
	if ((value = getenv("hostname")))
		sethostname(trim_r(value), strlen(value) + 1);
#endif
	if ((value = getenv("domain")))
		nvram_set(strcat_r(prefix, "domain", tmp), trim_r(value));
	if ((value = getenv("lease"))) {
		nvram_set(strcat_r(prefix, "lease", tmp), trim_r(value));
		expires(wan_ifname, atoi(value));
	}

	//logmessage("dhcp client", "%s IP : %s from %s", 
	//	udhcpstate, 
	//	nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
	//	nvram_safe_get(strcat_r(prefix, "gateway", tmp)));

	wanmessage("");

	dprintf("done\n");
	return 0;
}

int
udhcpc_main(int argc, char **argv)
{
	if (argv[1]) strcpy(udhcpstate, argv[1]);

	if (!argv[1])
		return EINVAL;
	else if (strstr(argv[1], "deconfig"))
		return deconfig();
	else if (strstr(argv[1], "bound"))
		return bound();
	else if (strstr(argv[1], "renew"))
		return renew();
	else if (strstr(argv[1], "leasefail"))
		return 0;
	else return deconfig();
}

