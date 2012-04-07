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
#ifndef FLASH2M
/*
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
#include <semaphore_mfp.h>

#include <signal.h>

static char udhcpstate[8];

static int
expires(char *lan_ifname, unsigned int in)
{
	time_t now;
	FILE *fp;
	char tmp[100];

	time(&now);
	snprintf(tmp, sizeof(tmp), "/tmp/udhcpc%d.expires", 0); 
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
	char *lan_ifname = safe_getenv("interface");
/*
	if (	nvram_match("sw_mode_ex", "2") &&
		!nvram_match("lan_ipaddr_t", "") && !nvram_match("lan_netmask_t", ""))
		ifconfig(lan_ifname, IFUP,
			 nvram_safe_get("lan_ipaddr_t"),
			 nvram_safe_get("lan_netmask_t"));
	else
*/
		ifconfig(lan_ifname, IFUP,
			 nvram_safe_get("lan_ipaddr"),
			 nvram_safe_get("lan_netmask"));

	expires(lan_ifname, 0);

	lan_down_ex(lan_ifname);

	logmessage("dhcp client", "%s: lease is lost", udhcpstate);
	//wanmessage("lost IP from server");

	dprintf("done\n");
	return 0;
}

/*
 * bound: This argument is used when udhcpc moves from an unbound, to
 * a bound state. All of the paramaters are set in enviromental
 * variables, The script should configure the interface, and set any
 * other relavent parameters (default gateway, dns server, etc).
*/
static int
bound(void)
{
	char *lan_ifname = safe_getenv("interface");
	char *value;
	char tmp[100], prefix[] = "lanXXXXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "lan_");

	if ((value = getenv("ip")))
		nvram_set(strcat_r(prefix, "ipaddr_t", tmp), value);
	if ((value = getenv("subnet")))
		nvram_set(strcat_r(prefix, "netmask_t", tmp), value);
	if ((value = getenv("router")))
		nvram_set(strcat_r(prefix, "gateway_t", tmp), value);
	if ((value = getenv("dns")))
		nvram_set(strcat_r(prefix, "dns_t", tmp), value);
	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins_t", tmp), value);
	//if ((value = getenv("hostname")))	// oleg patch del
	//	sethostname(value, strlen(value) + 1);
	if ((value = getenv("domain")))
		nvram_set(strcat_r(prefix, "domain_t", tmp), value);
	if ((value = getenv("lease"))) {
		nvram_set(strcat_r(prefix, "lease_t", tmp), value);
		expires(lan_ifname, atoi(value));
	}
		
	ifconfig(lan_ifname, IFUP,
		nvram_safe_get(strcat_r(prefix, "ipaddr_t", tmp)),
		nvram_safe_get(strcat_r(prefix, "netmask_t", tmp)));

	spinlock_lock(SPINLOCK_DHCPRenew);
	nvram_set("dhcp_renew", "0");
	spinlock_unlock(SPINLOCK_DHCPRenew);

	lan_up_ex(lan_ifname);

	logmessage("dhcp client", "%s IP: %s from %s", 
		udhcpstate, 
		nvram_safe_get(strcat_r(prefix, "ipaddr_t", tmp)), 
		nvram_safe_get(strcat_r(prefix, "gateway_t", tmp)));

	dprintf("done\n");

	return 0;
}

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
	bound();

	dprintf("done\n");
	return 0;
}
#if 0
static int
apply(void)	// step 2 of 2-step dhcp lease application for RT-N13 repeater mode UI
{
	char lan_ifname[5];
	char udhcpstate_local[8];

	if (	!nvram_match("sw_mode_ex", "2") ||
		!nvram_match("ui_triggered", "1") ||
		!nvram_get("lan_ipaddr_new") ||
		!nvram_get("lan_netmask_new") ||
		!nvram_get("lan_gateway_new"))
		return 0;

	kill_pidfile_s("/var/run/dhcpc_apply_delayed.pid", SIGTERM);	// kill pending process
	unlink("/var/run/dhcpc_apply_delayed.pid");

	nvram_set("ui_triggered", "0");

	nvram_set("lan_ipaddr_now", nvram_safe_get("lan_ipaddr_new"));
	nvram_set("lan_netmask_now", nvram_safe_get("lan_netmask_new"));
	nvram_set("lan_gateway_now", nvram_safe_get("lan_gateway_new"));
	nvram_set("lan_dns_now", nvram_safe_get("lan_dns_new"));
	nvram_set("lan_wins_now", nvram_safe_get("lan_wins_new"));
	nvram_set("lan_domain_now", nvram_safe_get("lan_domain_new"));
	nvram_set("lan_lease_now", nvram_safe_get("lan_lease_new"));
	strcpy(lan_ifname, nvram_safe_get("lan_ifname_new"));
	strcpy(udhcpstate_local, nvram_safe_get("lan_udhcpstate_new"));

	nvram_unset("lan_ipaddr_new");
	nvram_unset("lan_netmask_new");
	nvram_unset("lan_gateway_new");
	nvram_unset("lan_dns_new");
	nvram_unset("lan_wins_new");
	nvram_unset("lan_domain_new");
	nvram_unset("lan_lease_new");
	nvram_unset("lan_ifname_new");
	nvram_unset("lan_udhcpstate_new");

	nvram_set("lan_ipaddr_t", nvram_safe_get("lan_ipaddr_now"));
	nvram_set("lan_netmask_t", nvram_safe_get("lan_netmask_now"));
	nvram_set("lan_gateway_t", nvram_safe_get("lan_gateway_now"));
	nvram_set("lan_dns_t", nvram_safe_get("lan_dns_now"));
	nvram_set("lan_wins_t", nvram_safe_get("lan_wins_now"));
	nvram_set("lan_domain_t", nvram_safe_get("lan_domain_now"));
	nvram_set("lan_lease_t", nvram_safe_get("lan_lease_now"));

	system("mii_mgr -s -p 0 -r 0 -v 0x3900");	// link down switch port 0
	system("mii_mgr -s -p 1 -r 0 -v 0x3900");	// link down switch port 1
	system("mii_mgr -s -p 2 -r 0 -v 0x3900");	// link down switch port 2
	system("mii_mgr -s -p 3 -r 0 -v 0x3900");	// link down switch port 3
	system("mii_mgr -s -p 4 -r 0 -v 0x3900");	// link down switch port 4

	dbg("Apply lease %s/%s from %s to %s\n",
		nvram_safe_get("lan_ipaddr_t"),
		nvram_safe_get("lan_netmask_t"),
		nvram_safe_get("lan_gateway_t"),
		lan_ifname);

	ifconfig(lan_ifname, IFUP,
		nvram_safe_get("lan_ipaddr_t"),
		nvram_safe_get("lan_netmask_t"));

	lan_up_ex(lan_ifname);

	logmessage("dhcp client", "%s IP: %s from %s", 
		udhcpstate_local,
		nvram_safe_get("lan_ipaddr_t"),
		nvram_safe_get("lan_netmask_t"));

	//dbg("sleep for 1 second...\n");
	sleep(1);

	system("mii_mgr -s -p 0 -r 0 -v 0x3100");	// link up switch port 0
	system("mii_mgr -s -p 1 -r 0 -v 0x3100");	// link up switch port 1
	system("mii_mgr -s -p 2 -r 0 -v 0x3100");	// link up switch port 2
	system("mii_mgr -s -p 3 -r 0 -v 0x3100");	// link up switch port 3
	system("mii_mgr -s -p 4 -r 0 -v 0x3100");	// link up switch port 4

	return 0;
}
#endif
int
udhcpc_ex_main(int argc, char **argv)
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
	else if (strstr(argv[1], "leasefail"))	// oleg patch
		return 0;
#if 0
	else if (strstr(argv[1], "apply"))
		return apply();
#endif
	else return deconfig();
}
#endif
