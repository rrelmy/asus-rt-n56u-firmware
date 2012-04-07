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
 * Router rc control script
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

#ifndef _rc_h_
#define _rc_h_

//#include <bcmconfig.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define DEBUG_USB

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

#include <usb_info.h>

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

/* udhcpc scripts */
extern int udhcpc_main(int argc, char **argv);

/* ppp scripts */
extern int ipup_main(int argc, char **argv);
extern int ipdown_main(int argc, char **argv);
extern int ppp_ifunit(char *ifname);

/* http functions */
extern int http_get(const char *server, char *buf, size_t count, off_t offset);
extern int http_post(const char *server, char *buf, size_t count);
extern int http_stats(const char *url);

/* init */
extern int console_init(void);
extern pid_t run_shell(int timeout, int nowait);
extern void signal_init(void);
extern void fatal_signal(int sig);

/* interface */
extern in_addr_t inet_addr_(const char *cp);	// oleg patch

extern int ifconfig(char *ifname, int flags, char *addr, char *netmask);
extern int route_add(char *name, int metric, char *dst, char *gateway, char *genmask);
extern int route_del(char *name, int metric, char *dst, char *gateway, char *genmask);
extern void config_loopback(void);
extern int config_vlan(void);

/* network */
extern void start_lan(void);
extern void stop_lan(void);
extern void start_wan(void);
extern void stop_wan(void);
extern void wan_up(char *ifname);
extern void wan_down(char *ifname);
extern int hotplug_net(void);
extern int wan_ifunit(char *ifname);
extern int wan_primary_ifunit(void);
extern int update_resolvconf(void);
extern int add_dns(const char *ifname);
extern int del_dns(const char *ifname);

/* services */
extern int start_dhcpd(void);
extern void stop_dhcpd(void);
extern int start_dns(void);
extern void stop_dns(void);
extern int start_ntpc(void);
extern void stop_ntpc(void);
extern int start_services(void);
extern void stop_services(void);

/* firewall */
#ifdef __CONFIG_NETCONF__
extern int start_firewall(void);
extern int stop_firewall(void);
extern int start_firewall2(char *ifname);
extern int stop_firewall2(char *ifname);
#else
#define start_firewall() do {} while (0)
#define stop_firewall() do {} while (0)
#define start_firewall2(ifname) do {} while (0)
#define stop_firewall2(ifname) do {} while (0)
#endif

/* routes */
extern int preset_wan_routes(char *ifname);

#ifdef BTN_SETUP
enum BTNSETUP_STATE
{
	BTNSETUP_NONE=0,
	BTNSETUP_DETECT,
	BTNSETUP_START,
	BTNSETUP_DATAEXCHANGE,
	BTNSETUP_DATAEXCHANGE_FINISH,
	BTNSETUP_DATAEXCHANGE_EXTEND,
	BTNSETUP_FINISH
};
#endif

// for log message title
//#define LOGNAME	"RT-N56U"
#define LOGNAME	nvram_safe_get("productid")
#define ERR	"err"

// wl_guest	// ham 1031
#define WL_GUEST_IF_1	"wl0.1"

#define varkey_nvram_set(key, value, args...)({ \
        char nvram_word[64]; \
        memset(nvram_word, 0x00, sizeof(nvram_word)); \
        sprintf(nvram_word, key, ##args); \
        nvram_set(nvram_word, value); \
})

#define varval_nvram_set(key, value, args...)({ \
        char nvram_value[64]; \
        memset(nvram_value, 0x00, sizeof(nvram_value)); \
        sprintf(nvram_value, value, ##args); \
        nvram_set(key, nvram_value); \
})

#define MACSIZE 12
#define MAX_CONNTRACK_DM "16384"
#endif /* _rc_h_ */
