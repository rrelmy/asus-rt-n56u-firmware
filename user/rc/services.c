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
 * Miscellaneous services
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
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <nvram/bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include <semaphore_mfp.h>
#include "xstart.h"	// jerry5

#ifndef ASUS_EXT
int
start_dhcpd(void)
{
	if (nvram_match("router_disable", "1") || !nvram_match("lan_proto", "dhcp"))
		return 0;

	FILE *fp;
	char name[100];
	int dhcp_lease_time;


	/* Touch leases file */
	if (!(fp = fopen("/tmp/udhcpd.leases", "a"))) {
		perror("/tmp/udhcpd.leases");
		return errno;
	}
	fclose(fp);

	/* Write configuration file based on current information */
	if (!(fp = fopen("/tmp/udhcpd.conf", "w"))) {
		perror("/tmp/udhcpd.conf");
		return errno;
	}
	fprintf(fp, "pidfile /var/run/udhcpd.pid\n");
	fprintf(fp, "start %s\n", nvram_safe_get("dhcp_start"));
	fprintf(fp, "end %s\n", nvram_safe_get("dhcp_end"));
	fprintf(fp, "interface %s\n", nvram_safe_get("lan_ifname"));
	fprintf(fp, "remaining yes\n");
	fprintf(fp, "lease_file /tmp/udhcpd.leases\n");
	fprintf(fp, "option subnet %s\n", nvram_safe_get("lan_netmask"));
	fprintf(fp, "option router %s\n", nvram_safe_get("lan_ipaddr"));
	fprintf(fp, "option dns %s\n", nvram_safe_get("lan_ipaddr"));
	dhcp_lease_time = atoi(nvram_safe_get(dhcp_lease));
	if (dhcp_lease_time <= 3)
		dhcp_lease_time = 86400;
	fprintf(fp, "option lease %d\n", dhcp_lease_time);
	snprintf(name, sizeof(name), "%s_wins", nvram_safe_get("dhcp_wins"));
	if (!nvram_match(name, ""))
		fprintf(fp, "option wins %s\n", nvram_safe_get(name));
	snprintf(name, sizeof(name), "%s_domain", nvram_safe_get("dhcp_domain"));
	if (!nvram_match(name, ""))
		fprintf(fp, "option domain %s\n", nvram_safe_get(name));
	fclose(fp);

	char *dhcpd_argv[] = {"udhcpd", "/tmp/udhcpd.conf", NULL};
	int upid;
	_eval(dhcpd_argv, NULL, 0, &upid);

	return 0;
}

void
stop_dhcpd(void)
{
/*
* Process udhcpd handles two signals - SIGTERM and SIGUSR1
*
*  - SIGUSR1 saves all leases in /tmp/udhcpd.leases
*  - SIGTERM causes the process to be killed
*
* The SIGUSR1+SIGTERM behavior is what we like so that all current client
* leases will be honorred when the dhcpd restarts and all clients can extend
* their leases and continue their current IP addresses. Otherwise clients
* would get NAK'd when they try to extend/rebind their leases and they 
* would have to release current IP and to request a new one which causes 
* a no-IP gap in between.
*/
	if (pids("udhcpd"))
	{
		doSystem("killall -%d udhcpd", SIGUSR1);
		sleep(1);
		doSystem("killall udhcpd");
	}

	dprintf("done\n");
}

#endif	// ASUS_EXT

int
chpass(char *user, char *pass)
{
	char cmdbuf[128];
	FILE *fp;

	if (!user)
		user = "admin";

	if (!pass)
		pass = "admin";

	if ((fp = fopen("/etc/passwd", "a")))
	{
		fprintf(fp, "%s::0:0::/:\n", user);
		fclose(fp);
	}

	if ((fp = fopen("/etc/group", "a")))
	{
		fprintf(fp, "%s:x:0:%s\n", user, user);
		fclose(fp);
	}

	memset(cmdbuf, 0x0, 128);
	sprintf(cmdbuf, "chpasswd.sh %s %s", user, pass);
	system(cmdbuf);

/*
	memset(cmdbuf, 0, sizeof(cmdbuf));
	sprintf(cmdbuf, "echo \"%s::0:0:Adminstrator:/:/bin/sh\" > /etc/passwd", user);
	system(cmdbuf);

	memset(cmdbuf, 0, sizeof(cmdbuf));
	sprintf(cmdbuf, "echo \"%s:x:0:%s\" > /etc/group", user, user);
	system(cmdbuf);

	memset(cmdbuf, 0, sizeof(cmdbuf));
	sprintf(cmdbuf, "chpasswd.sh %s %s", user, pass);
	system(cmdbuf);
*/
	return 0;
}

int 
start_telnetd()
{
	char *telnetd_argv[] = {"telnetd", NULL};

	if (!nvram_match("telnetd", "1"))
		return 0;

	if (pids("telnetd"))
		system("killall telnetd");

	chpass(nvram_safe_get("http_username"), nvram_safe_get("http_passwd"));	// vsftpd also needs

	return system("telnetd");
}

void 
stop_telnetd()
{
	if (pids("telnetd"))
		system("killall telnetd");
}

int 
run_telnetd()
{
	char *telnetd_argv[] = {"telnetd", NULL};

	if (pids("telnetd"))
		system("killall telnetd");

	chpass(nvram_safe_get("http_username"), nvram_safe_get("http_passwd"));	// vsftpd also needs

	return system("telnetd");
}

int
start_httpd(void)
{
	int ret;

	chdir("/www");

	if (nvram_match("wan_route_x", "IP_Routed"))
	{
#ifdef RTCONFIG_USB_MODEM
		if(get_usb_modem_state()){
#ifdef RTCONFIG_USB_MODEM_WIMAX
			if(nvram_match("modem_enable", "4")){
				char cmd[64];
				memset(cmd, 0, 64);
				sprintf(cmd, "httpd %s &", WIMAX_INTERFACE);
				ret = system(cmd);
			}
			else
#endif
				ret = system("httpd ppp0 &");
		}
		else
#endif
		if (nvram_match("wan0_proto", "dhcp") || nvram_match("wan0_proto", "static"))
			ret = system("httpd eth3 &");
		else
			ret = system("httpd ppp0 &");
	}
	else
		ret = system("httpd &");

	chdir("/");

//	logmessage(LOGNAME, "start httpd");

	return ret;
}

void
stop_httpd(void)
{
	if (pids("httpd"))
		system("killall httpd");
}
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
//////////vvvvvvvvvvvvvvvvvvvvvjerry5 2009.07
void
stop_rstats(void)
{
	if (pids("rstats"))
		system("killall rstats");
}

void
start_rstats(int new)
{
        if (nvram_match("wan_route_x", "IP_Routed") && nvram_match("rstats_enable", "1")) {
                stop_rstats();
                if (new)
//			xstart("rstats", "--new");
			system("rstats --new");
                else
//			xstart("rstats");
			system("rstats");
        }
}

void
restart_rstats()
{
	if (nvram_match("rstats_bak", "1"))
	{
		nvram_set("rstats_path", "*nvram");
		if (nvram_match("rstats_new", "1"))
		{
			start_rstats(1);
			nvram_set("rstats_new", "0");
		}
		else
			start_rstats(0);
	}
	else 
	{
		nvram_set("rstats_path", "");
		start_rstats(0);
	}
}
////////^^^^^^^^^^^^^^^^^^^jerry5 2009.07
#endif
int 
start_upnp()
{
	char cmdbuf[64];
	char *wan_proto;

#ifdef WIFI_LOGO
	return 0;
#endif

	if (nvram_match("upnp_enable", "0") || nvram_match("router_disable", "1"))
		return 0;

	system("route add -net 239.0.0.0 netmask 255.0.0.0 dev br0");
	memset(cmdbuf, 0, sizeof(cmdbuf));
	sprintf(cmdbuf, "upnp_xml.sh %s %s", nvram_safe_get("lan_ipaddr_t"), nvram_safe_get("br0hexaddr"));
	system(cmdbuf);

	wan_proto = nvram_safe_get("wan_proto");
#ifdef RTCONFIG_USB_MODEM
	if(get_usb_modem_state()){
#ifdef RTCONFIG_USB_MODEM_WIMAX
		if(nvram_match("modem_enable", "4")){
			char cmd[64];
			memset(cmd, 0, 64);
			sprintf(cmd, "upnpd -f %s br0 &", WIMAX_INTERFACE);
			system(cmd);
		}
		else
#endif
			system("upnpd -f ppp0 br0 &");
	}
	else
#endif
	if (strcmp(wan_proto, "pppoe") == 0 || 
	    strcmp(wan_proto, "pptp") == 0 || 
	    strcmp(wan_proto, "l2tp") == 0)
	{
		system("upnpd -f ppp0 br0 &");
	}
	else
	{
		system("upnpd -f eth3 br0 &");
	}

	nvram_set("upnp_started", "1");

	return 0;
}

void
stop_upnp(void)
{
	if (pids("upnpd"))
	{
		kill_pidfile_s("/var/run/upnpd.pid", SIGTERM);
		sleep(2);
	}
}

int
start_ntpc(void)
{
#ifdef ASUS_EXT
	if (pids("ntpclient"))
		system("killall ntpclient");

	if (!pids("ntp"))
		system("ntp &");
#else
	char *servers = nvram_safe_get("ntp_server");
	
	if (strlen(servers)) {
		char *nas_argv[] = {"/usr/sbin/ntpclient", "-h", servers, "-i", "3", "-l", "-s", NULL};
		_eval(nas_argv, NULL, 0, NULL);
	}
	
#endif
	return 0;
}

void
stop_ntpc(void)
{
//	if (nvram_match("wan_nat_x", "0"))
//		return 0;
#ifdef ASUS_EXT
	if (pids("ntp"))
		system("killall -SIGTERM ntp");
	if (pids("ntpclient"))
		system("killall ntpclient");
#else
	if (pids("ntpclient"))
		system("killall ntpclient");
#endif
}

int start_lltd(void)
{
	system("lld2d br0");
	
	return 0;
}

void stop_lltd(void)
{
	if (pids("lld2d"))
		system("killall lld2d");
}

int 
start_lpd()
{
	if (!pids("pids"))
	{
		unlink("/var/run/lpdparent.pid");
		return system("lpd &");
	}
	else
		return -1;
}

void
stop_lpd()
{
	int delay_count = 10;

	if (pids("lpd"))
	{
		system("killall lpd");
		unlink("/var/run/lpdparent.pid");
        
		while (pids("lpd") && (delay_count-- > 0))
			sleep(1);

	}
}

void init_spinlock()
{
//	spinlock_init(SPINLOCK_SiteSurvey);
	spinlock_init(SPINLOCK_NVRAMCommit);
	spinlock_init(SPINLOCK_DHCPRenew);
	spinlock_lock(SPINLOCK_DHCPRenew);
	nvram_set("dhcp_renew", "0");
	spinlock_unlock(SPINLOCK_DHCPRenew);
}

void nvram_commit_safe()
{
	spinlock_lock(SPINLOCK_NVRAMCommit);
	nvram_commit();
	spinlock_unlock(SPINLOCK_NVRAMCommit);
}

int
start_services(void)
{
	printf("[rc] start services\n");

	start_syslogd();

	logmessage("RT-N56U", "bootloader version: %s", nvram_safe_get("blver"));
	logmessage("RT-N56U", "firmware version: %s", nvram_safe_get("firmver_sub"));

	if (!nvram_match("computer_name", "") && is_valid_hostname(nvram_safe_get("computer_name")))
		doSystem("hostname %s", nvram_safe_get("computer_name"));
	else
		doSystem("hostname %s", nvram_safe_get("productid"));

        start_telnetd();
	start_klogd();
	start_dns();
	start_8021x();
	start_8021x_rt();
	start_infosvr();
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	start_ots();
	start_detect_internet();
#endif
	start_httpd();
	start_watchdog();

	if (nvram_match("lan_stp", "1") && !is_ap_mode())
	{
		dbg("resume stp forwarding delay and hello time\n");
		system("brctl setfd br0 15");
		system("brctl sethello br0 2");
	}

	start_networkmap();
#ifndef WIFI_LOGO
	start_lltd();
#endif

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
#ifdef WEB_REDIRECT
	if (    nvram_match("wan_route_x", "IP_Routed") &&
		nvram_match("wan_nat_x", "1") &&
		nvram_match("wan_pppoe_relay_x", "0")   )
	{
		printf("--- START: Wait to start wanduck ---\n");
		redirect_setting();
		start_wanduck();
	}
#endif
	restart_rstats();
#endif

#ifdef WSC
	if (nvram_match("wps_band", "0"))
	{
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	if (nvram_match("wsc_config_state", "0") && !nvram_match("wl_radio_x", "0") && !nvram_match("sw_mode_ex", "3"))
											// psp fix
#else
	if (!nvram_match("sw_mode_ex", "3"))						// psp fix
#endif
	{
		start_wsc_pin_enrollee();
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
		start_pspfix();								// psp fix
#endif
	}
	else										// psp fix
	{
		nvram_set("wps_enable", "0");
		nvram_set("wps_start_flag", "0");
	}
	}
	else
	{
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	if (nvram_match("rt_wsc_config_state", "0") && !nvram_match("rt_radio_x", "0")  && !nvram_match("sw_mode_ex", "3"))
											// psp fix
#else
	if (!nvram_match("sw_mode_ex", "3"))						// psp fix
#endif
	{
		start_wsc_pin_enrollee_2g();
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
		start_pspfix();								// psp fix
#endif
	}
	else										// psp fix
	{
		nvram_set("wps_enable", "0");
		nvram_set("wps_start_flag", "0");
	}
	}

#endif

#if 0
	if (nvram_match("sw_mode_ex", "2"))
	{
		start_apcli_monitor();
		start_ping_keep_alive();
	}
#endif

	if (is_RT3090_loaded())
	nvram_set("success_start_service", "1");	// For judging if the system is ready.

	return 0;
}

void
stop_logger(void)
{
	if (pids("klogd"))
		system("killall klogd");
	if (pids("syslogd"))
		system("killall syslogd");
}

void
stop_services(void)
{
	stop_upnp();
#ifdef ASUS_EXT
	stop_logger();
#endif
#if 0
#ifdef GUEST_ACCOUNT
	stop_dhcpd_guest();
#endif
#endif	// #if 0
	stop_dhcpd();
//	stop_dns();
//	stop_httpd();

	stop_lpd();
	stop_u2ec();
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	stop_rstats();		// jerry5
#endif
}
#if 0
#ifdef GUEST_ACCOUNT
/* Start NAS for the guest interfaces */
int
start_guest_nas(void)
{
	char *unbridged_interfaces;
	char *next;
	char name[IFNAMSIZ],lan[IFNAMSIZ];
	int index;
		
	unbridged_interfaces = nvram_get("unbridged_ifnames");
	
	if (unbridged_interfaces)
		foreach(name,unbridged_interfaces,next) {
			index = get_ipconfig_index(name);
			if (index < 0) 
				continue;
			snprintf(lan,sizeof(lan),"lan%d",index);
			start_nas(lan);
		}

	return 0;
}
#endif
#endif 	// #if 0
// 2008.10 magic {
#ifdef WEB_REDIRECT
int start_wanduck(void)
{
#ifdef WIFI_LOGO
	return 0;
#endif

	if (nvram_match("wan_route_x", "IP_Bridged"))
		return -1;
	else if (nvram_match("wanduck_down", "1"))
		return -1;
	else if	(nvram_match("wan_pppoe_relay_x", "1"))
		return -1;
	
	char *argv[] = {"/sbin/wanduck", NULL};
	int ret = 0;
	pid_t pid;
	FILE *fp = fopen("/var/run/wanduck.pid", "r");
	char *wan_proto_type = nvram_safe_get("wan0_proto");
	int i;

	if (fp != NULL) {
		fclose(fp);
		return 0;
	}

	if (wan_proto_type && (strcmp(wan_proto_type, "pptp") || strcmp(wan_proto_type, "l2tp"))) // delay run
	{
		printf("\nDelay run wanduck 3 seconds\n");
		sleep(3);
		ret = _eval(argv, NULL, 0, &pid);
	}
	else
        {
                printf("\nDelay run wanduck 5 seconds\n");
                sleep(5);
                ret = _eval(argv, NULL, 0, &pid);
        }

	return ret;
}

void stop_wanduck(void)
{
//	kill_pidfile_s("/var/run/wanduckmain.pid", SIGUSR2);
	
	kill_pidfile_s("/var/run/wanduck.pid", SIGTERM);
}
#endif
