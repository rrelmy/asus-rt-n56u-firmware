/*
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifdef ASUS_EXT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <sys/mount.h>
#include <nvram/bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include <syslog.h>
#include "iboxcom.h"
#include "lp.h"
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <sys/mount.h>

#include <rtxxxx.h>
#include <rc_event.h>
#include <dongles.h>

#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <scsi/scsi.h>
#include <scsi/scsi_ioctl.h>

#ifdef WCN
#include "ILibParsers.h"
#include <wlutils.h>
#endif

#ifdef DLM
#include <asm/page.h>
#include <sys/swap.h>
#endif

#include <linux/autoconf.h>
#include "../../config/autoconf.h"

#define logs(s) syslog(LOG_NOTICE, s)

char *usb_dev_file = "/proc/bus/usb/devices";

#define USB_CLS_PER_INTERFACE	0	/* for DeviceClass */
#define USB_CLS_AUDIO		1
#define USB_CLS_COMM		2
#define USB_CLS_HID		3
#define USB_CLS_PHYSICAL	5
#define USB_CLS_STILL_IMAGE	6
#define USB_CLS_PRINTER		7
#define USB_CLS_MASS_STORAGE	8
#define USB_CLS_HUB		9
#define USB_CLS_CDC_DATA	0x0a
#define USB_CLS_CSCID		0x0b	/* chip+ smart card */
#define USB_CLS_CONTENT_SEC	0x0d	/* content security */
#define USB_CLS_VIDEO		0x0e
#define USB_CLS_WIRELESS_CONTROLLER	0xe0
#define USB_CLS_MISC		0xef
#define USB_CLS_APP_SPEC	0xfe
#define USB_CLS_VENDOR_SPEC	0xff
#define USB_CLS_3GDEV		0x35

#define OP_MOUNT		1
#define OP_UMOUNT		2
#define OP_SETNVRAM		3

//#define NO_DM			1	// J++

#ifdef DLM
/* CRC lookup table */
static unsigned long crcs[256]={ 0x00000000,0x77073096,0xEE0E612C,0x990951BA,
0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,
0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,
0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,
0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,
0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,
0xDCD60DCF,0xABD13D59,0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,
0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,
0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,0x76DC4190,0x01DB7106,0x98D220BC,
0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,
0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,0x6B6B51F4,
0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,
0xFBD44C65,0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,
0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,
0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,
0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,
0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,
0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,
0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,
0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,
0x17B7BE43,0x60B08ED5,0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,
0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,
0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,
0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,
0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,
0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,
0x0BDBDF21,0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,
0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,
0xF862AE69,0x616BFFD3,0x166CCF45,0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,
0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,
0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,
0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,
0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D};
#endif

#ifdef USB_SUPPORT
//#define WEBCAM_SUPPORT 1
#define PRINTER_SUPPORT 1
#define MASSSTORAGE_SUPPORT 1
//#define AUDIO_SUPPORT 1

enum
{
	WEB_NONE = 0,
	WEB_PWCWEB,
	WEB_OVWEB,
//	WEB_AUDIO
} WEBTYPE;

char *PWCLIST[] = {"471","69a","46d","55d","41e","4cc","d81", NULL};
char *OVLIST[] = {"5a9","813","b62", NULL};

#endif

char buf_g[512];

void init_apps();
void run_ftp();
void exec_apps();
int check_task(char *cmd);
void chk_partitions(int state);
void stop_samba();
void stop_ftp();
void run_ftpsamba();
void run_samba();
void run_dms();
void stop_networkmap();
//int process_check_by_pidfile(const char *pid_file);
void stop_dms();
#ifdef CONFIG_USER_MTDAAPD
void stop_mt_daapd();
#endif
void umount_ejected();
void umount_usb_path(char *port);
char *find_sddev_by_mountpoint(char *mountpoint);
int count_sddev_mountpoint();
int count_sddev_partition();
void restart_apps();
void umount_sddev_all();
int is_valid_hostname(const char *name);

int file_to_buf(char *path, char *buf, int len)
{
	FILE *fp;

	memset(buf, 0 , len);

	if ((fp = fopen(path, "r"))) {
		fgets(buf, len, fp);
		fclose(fp);
		return 1;
	}

	return 0;
}

void
stop_infosvr()
{
	if (pids("infosvr"))
		system("killall infosvr");
}

int 
start_infosvr()
{
	char *infosvr_argv[] = {"/usr/sbin/infosvr", "br0", NULL};
	pid_t pid;

	return _eval(infosvr_argv, NULL, 0, &pid);
}

int
start_8021x()
{
//	char *apd_argv[] = {"/bin/rt2860apd",/* "-i", WIF, */NULL};
//	pid_t pid;

	if (	nvram_match("wl_auth_mode", "wpa") || 
		nvram_match("wl_auth_mode", "radius") || 
		nvram_match("wl_auth_mode", "wpa2") )
	{
		if (pids("rt2860apd"))
			system("killall rt2860apd");

		dbg("8021X daemon for 5G...\n");
//		return _eval(apd_argv, NULL, 0, &pid);
		return doSystem("rt2860apd");
	}
	else
		return 0;
}

int
start_8021x_rt()
{
//	char *apd_argv[] = {"/bin/rtinicapd",/* "-i", WIF2G, */NULL};
//	pid_t pid;

	if (	nvram_match("rt_auth_mode", "wpa") || 
		nvram_match("rt_auth_mode", "radius") || 
		nvram_match("rt_auth_mode", "wpa2") )
	{
		if (pids("rtinicapd"))
			system("killall rtinicapd");

		dbg("8021X daemon for 2.4G...\n");
//		return _eval(apd_argv, NULL, 0, &pid);
		return doSystem("rtinicapd");
	}
	else
		return 0;
}

int
start_dhcpd(void)
{
	FILE *fp;
	char *slease = "/tmp/udhcpd-br0.sleases";
	char word[256], *next;
//	int auto_dns = 0;

	if (!nvram_match("dhcp_enable_x", "1"))	return 0;

	if (	nvram_match("sw_mode_ex", "3") ||
		((nvram_match("sw_mode_ex", "1") || nvram_match("sw_mode_ex", "4")) && !nvram_match("lan_proto", "dhcp")))
	{
		dbg("skip running udhcpd...\n");
		return 0;
	}
	else
		dbg("starting udhcpd...\n");

	dprintf("%s %s %s %s\n",
		nvram_safe_get("lan_ifname"),
		nvram_safe_get("dhcp_start"),
		nvram_safe_get("dhcp_end"),
		//nvram_safe_get("lan_lease"));
		nvram_safe_get("dhcp_lease"));

	if (!(fp = fopen("/tmp/udhcpd-br0.leases", "a"))) {
		perror("/tmp/udhcpd-br0.leases");
		return errno;
	}
	fclose(fp);

	// Write configuration file based on current information
	if (!(fp = fopen("/tmp/udhcpd.conf", "w"))) {
		perror("/tmp/udhcpd.conf");
		return errno;
	}
	
	//fprintf(fp, "pidfile /var/run/udhcpd-br0.pid\n");
	fprintf(fp, "start %s\n", nvram_safe_get("dhcp_start"));
	fprintf(fp, "end %s\n", nvram_safe_get("dhcp_end"));
	//fprintf(fp, "interface %s\n", nvram_safe_get("lan_ifname"));
	fprintf(fp, "interface br0\n");
	fprintf(fp, "remaining yes\n");
	fprintf(fp, "lease_file /tmp/udhcpd-br0.leases\n");
	fprintf(fp, "option subnet %s\n", nvram_safe_get("lan_netmask"));
	
	if (!nvram_match("dhcp_gateway_x", ""))
		fprintf(fp, "option router %s\n", nvram_safe_get("dhcp_gateway_x"));	
	else	
		fprintf(fp, "option router %s\n", nvram_safe_get("lan_ipaddr"));	

	if (!nvram_match("dhcp_dns1_x", ""))
	{
		fprintf(fp, "option dns %s\n", nvram_safe_get("dhcp_dns1_x"));
		logmessage("dhcpd", "add option dns: %s", nvram_safe_get("dhcp_dns1_x"));
	}
	fprintf(fp, "option dns %s\n", nvram_safe_get("lan_ipaddr"));
	logmessage("dhcpd", "add option dns: %s", nvram_safe_get("lan_ipaddr"));
#if 0
        if (!nvram_match("wan0_proto", "static") && nvram_match("wan0_dnsenable_x", "1"))
	{
		foreach(word, (strlen(nvram_safe_get("wan0_dns")) ? nvram_safe_get("wan0_dns") : nvram_safe_get("wanx_dns")), next)
		{
			auto_dns++;
			fprintf(fp, "option dns %s\n", word);
			logmessage("dhcpd", "add option dns: %s", word);
		}

		if (!auto_dns && (nvram_match("dhcp_dns1_x", "") || !nvram_match("dhcp_dns1_x", "8.8.8.8")))
		{
			fprintf(fp, "option dns 8.8.8.8\n");
			logmessage("dhcpd", "add option dns: %s", "8.8.8.8");
		}
	}
#endif
	fprintf(fp, "option lease %s\n", nvram_safe_get("dhcp_lease"));

	if (!nvram_match("dhcp_wins_x", ""))		
		fprintf(fp, "option wins %s\n", nvram_safe_get("dhcp_wins_x"));		
	if (!nvram_match("lan_domain", ""))
		fprintf(fp, "option domain %s\n", nvram_safe_get("lan_domain"));
	fclose(fp);

	if (nvram_match("dhcp_static_x","1"))
	{	
		write_static_leases(slease);
		doSystem("/usr/sbin/udhcpd /tmp/udhcpd.conf /tmp/udhcpd-br0.sleases");
	}
	else
	{
		doSystem("/usr/sbin/udhcpd /tmp/udhcpd.conf");
	}

	return 0;
}

void
stop_dhcpd(void)
{
	int delay_count = 10;

	if (pids("udhcpd"))
	{
		doSystem("killall -%d udhcpd", SIGUSR1);
		sleep(1);
		doSystem("killall udhcpd");
	}
	else
		return;

	while (pids("udhcpd") && (delay_count-- > 0))
		sleep(1);
}

// dns patch check 0524
int
restart_dhcpd()
{
	dbg("restart udhcpd\n");	//tmp test

	stop_dhcpd();
	sleep(1);
	start_dhcpd();

	return 0;
}
// dns patch check 0524 end

extern int valid_url_filter_time();

int
start_dns(void)
{
	FILE *fp;

	if (pids("dproxy"))
		return restart_dns();

	if (nvram_match("router_disable", "1"))
		return 0;

	/* Create resolv.conf with empty nameserver list */
	if (!(fp = fopen("/tmp/resolv.conf", "r")))
	{
		if (!(fp = fopen("/tmp/resolv.conf", "w"))) 
		{
			perror("/tmp/resolv.conf");
			return errno;
		}
		else fclose(fp);
	}
	else fclose(fp);

	if (!(fp = fopen("/tmp/dproxy.conf", "w")))
	{
		perror("/tmp/dproxy.conf");
		return errno;
	}

	fprintf(fp, "name_server=\n");
	fprintf(fp, "ppp_detect=0\n");
	fprintf(fp, "purge_time=1200\n");
//	fprintf(fp, "deny_file=/tmp/dproxy.deny\n");
	fprintf(fp, "deny_file=\n");
	fprintf(fp, "cache_file=/tmp/dproxy.cache\n");
	fprintf(fp, "hosts_file=/tmp/hosts\n");
	fprintf(fp, "dhcp_lease_file=\n");
	fprintf(fp, "ppp_dev=/var/run/ppp0.pid\n");
	fprintf(fp, "debug_file=/tmp/dproxy.log\n");
	fclose(fp);

	// if user want to set dns server by himself
	if (nvram_match("wan0_proto", "static") || !nvram_match("wan0_dnsenable_x", "1"))
	{
		/* Write resolv.conf with upstream nameservers */
		if (!(fp = fopen("/tmp/resolv.conf", "w")))
		{
			perror("/tmp/resolv.conf");
			return errno;
		}
	
		if (!nvram_match("wan_dns1_x", ""))
			fprintf(fp, "nameserver %s\n", nvram_safe_get("wan_dns1_x"));		
		if (!nvram_match("wan_dns2_x", ""))
			fprintf(fp, "nameserver %s\n", nvram_safe_get("wan_dns2_x"));
		fclose(fp);
	}

/*
	active = timecheck_item(nvram_safe_get("url_date_x"), nvram_safe_get("url_time_x"));
	active1 = timecheck_item(nvram_safe_get("url_date_x"), nvram_safe_get("url_time_x_1"));
	if (valid_url_filter_time() && (active || active1))
	{
		if (!(fp = fopen("/tmp/dproxy.deny", "w")))
		{
			perror("/tmp/dproxy.deny");
			return errno;
		}

		for (i=0; i<atoi(nvram_safe_get("url_num_x")); i++)
		{
			sprintf(word, "url_keyword_x%d", i);
			fprintf(fp, "%s\n", nvram_safe_get(word));
		}
	
		fclose(fp);	
	}
	else
		unlink("/tmp/dproxy.deny");
*/
	if (!(fp = fopen("/tmp/hosts", "w")))
	{
		perror("/tmp/hosts");
		return errno;
	}

	fprintf(fp, "127.0.0.1 localhost.localdomain localhost\n");
	fprintf(fp, "%s	my.router\n", nvram_safe_get("lan_ipaddr"));
	fprintf(fp, "%s	my.%s\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("productid"));
	fprintf(fp, "%s %s\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("productid"));

	if (strcmp(nvram_safe_get("productid"), nvram_safe_get("computer_name")) && is_valid_hostname(nvram_safe_get("computer_name")))
		fprintf(fp, "%s %s\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("computer_name"));

	if (!nvram_match("lan_hostname", ""))
	{
		fprintf(fp, "%s %s.%s %s\n", nvram_safe_get("lan_ipaddr"),
					nvram_safe_get("lan_hostname"),
					nvram_safe_get("lan_domain"),
					nvram_safe_get("lan_hostname"));
	}	

	fclose(fp);	
		
	return system("dproxy -c /tmp/dproxy.conf");
}	

void
stop_dns(void)
{
	dbg("stop_dns()\n");

	int delay_count = 10;

	if (pids("dproxy"))
		system("killall -SIGKILL dproxy");
	else
		return;

	while (pids("dproxy") && (delay_count-- > 0))
		sleep(1);

//	unlink("/tmp/dproxy.deny");
}

int 
restart_dns()
{
	FILE *fp = NULL;

	if (pids("dproxy") && (fp = fopen("/tmp/dproxy.conf", "r")))
	{
		fclose(fp);
		return system("killall -SIGHUP dproxy");
	}

	stop_dns();

	return start_dns();
}

extern int ddns_timer;

int
ddns_updated_main(int argc, char *argv[])
{
	FILE *fp;
	char buf[64], *ip;

	if (!(fp=fopen("/tmp/ddns.cache", "r"))) return 0;
	
	fgets(buf, sizeof(buf), fp);
	fclose(fp);

	if (!(ip=strchr(buf, ','))) return 0;

	nvram_set("ddns_updated", "1");
	ddns_timer = 1;
	
	nvram_set("ddns_cache", buf);
	nvram_set("ddns_ipaddr", ip+1);
	nvram_set("ddns_status", "1");
	nvram_set("ddns_server_x_old", nvram_safe_get("ddns_server_x"));
	nvram_set("ddns_hostname_x_old", nvram_safe_get("ddns_hostname_x"));
	nvram_commit_safe();

	unlink("/tmp/dproxy.cache");

	logmessage("ddns", "ddns update ok");

	dprintf("done\n");

	return 0;
}

int 
start_ddns(void)
{
	FILE *fp = NULL;
	char *wan_ip, *ddns_cache;
	char server[32];
	char user[32];
	char passwd[32];
	char host[64];
	char service[32];
	char usrstr[64];
	char wan_ifname[16];
	int  wild=nvram_match("ddns_wildcard_x", "1");

	if (nvram_match("router_disable", "1")) return -1;
	
	if (!nvram_match("ddns_enable_x", "1")) return -1;
	
	if (!(wan_ip = nvram_safe_get("wan_ipaddr_t")) || nvram_match("wan_ipaddr_t", "")) return -1;

	if (	nvram_match("ddns_ipaddr", wan_ip) &&
		(/*nvram_match("ddns_server_x_old", "") ||*/
		!strcmp(nvram_safe_get("ddns_server_x"), nvram_safe_get("ddns_server_x_old"))) &&
		!strcmp(nvram_safe_get("ddns_hostname_x"), nvram_safe_get("ddns_hostname_x_old"))
	)
	{
		nvram_set("ddns_updated", "1");
		logmessage("ddns", "IP address has not changed since the last update");
		return -1;
	}

	if (	(inet_addr(wan_ip) == inet_addr(nvram_safe_get("ddns_ipaddr"))) &&
		(/*nvram_match("ddns_server_x_old", "") ||*/
		!strcmp(nvram_safe_get("ddns_server_x"), nvram_safe_get("ddns_server_x_old"))) &&
		!strcmp(nvram_safe_get("ddns_hostname_x"), nvram_safe_get("ddns_hostname_x_old"))
	)
	{
		nvram_set("ddns_updated", "1");
		logmessage("ddns", "IP address has not changed since the last update.");
		return -1;
	}

	// TODO : Check /tmp/ddns.cache to see current IP in DDNS
	// update when,
	// 	1. if ipaddr!= ipaddr in cache
	// 	
	// update
	// * nvram ddns_cache, the same with /tmp/ddns.cache

	if (    !nvram_match("ddns_server_x_old", "") &&
		strcmp(nvram_safe_get("ddns_server_x"), nvram_safe_get("ddns_server_x_old")) &&
		!nvram_match("ddns_hostname_x_old", "") &&
		!strcmp(nvram_safe_get("ddns_hostname_x"), nvram_safe_get("ddns_hostname_x_old"))
	)
	{
		logmessage("ddns", "clear ddns cache file for server setting change");
		unlink("/tmp/ddns.cache");
	}
	else if (!(fp = fopen("/tmp/ddns.cache", "r")) && (ddns_cache = nvram_get("ddns_cache")))
	{
		if ((fp = fopen("/tmp/ddns.cache", "w+")))
		{
			fprintf(fp, "%s", ddns_cache);
			fclose(fp);
		}
	}
	else
	{
		if (fp) fclose(fp);
	}

	strcpy(server, nvram_safe_get("ddns_server_x"));
	strcpy(user, nvram_safe_get("ddns_username_x"));
	strcpy(passwd, nvram_safe_get("ddns_passwd_x"));
	strcpy(host, nvram_safe_get("ddns_hostname_x"));
	strcpy(service, "");

	if (strcmp(server, "WWW.DYNDNS.ORG")==0)
		strcpy(service, "dyndns");			
	else if (strcmp(server, "WWW.DYNDNS.ORG(CUSTOM)")==0)
		strcpy(service, "dyndns");			
	else if (strcmp(server, "WWW.DYNDNS.ORG(STATIC)")==0)
		strcpy(service, "dyndns");			
	else if (strcmp(server, "WWW.TZO.COM")==0)
		strcpy(service, "tzo");			
	else if (strcmp(server, "WWW.ZONEEDIT.COM")==0)
		strcpy(service, "zoneedit");
	else if (strcmp(server, "WWW.JUSTLINUX.COM")==0)
		strcpy(service, "justlinux");
	else if (strcmp(server, "WWW.EASYDNS.COM")==0)
		strcpy(service, "easydns");
#ifdef ASUS_DDNS //2007.03.20 Yau add
	else if (strcmp(server, "WWW.ASUS.COM")==0)
		strcpy(service, "dyndns");
#endif
	else strcpy(service, "dyndns");

	sprintf(usrstr, "%s:%s", user, passwd);

	if (	nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "pptp") ||
		nvram_match("wan_proto", "l2tp")
#ifndef RTCONFIG_USB_MODEM
			|| nvram_match("wan_proto", "3g")
#else
			|| get_usb_modem_state()
#endif
			)	// oleg patch

	{
		strcpy(wan_ifname, nvram_safe_get("wan0_pppoe_ifname"));
	}
	else
	{
		strcpy(wan_ifname, nvram_safe_get("wan0_ifname"));
	}	

	dbg("wan_ifname: %s\n\n\n\n", wan_ifname);	// tmp test

	nvram_set("bak_ddns_enable_x", nvram_safe_get("ddns_enable_x"));
	nvram_set("bak_ddns_server_x", nvram_safe_get("ddns_server_x"));
	nvram_set("bak_ddns_username_x", nvram_safe_get("ddns_username_x"));
	nvram_set("bak_ddns_passwd_x", nvram_safe_get("ddns_passwd_x"));
	nvram_set("bak_ddns_hostname_x", nvram_safe_get("ddns_hostname_x"));
	nvram_set("bak_ddns_wildcard_x", nvram_safe_get("ddns_wildcard_x"));

#ifdef ASUS_DDNS //2007.03.20 Yau add
	if (!strcmp(server, "WWW.ASUS.COM"))
	{
		dprintf("ddns update %s %s\n", server, service);

		if (pids("ez-ipupdate"))
		{
			system("killall -SIGINT ez-ipupdate");
			sleep(1);
		}

		doSystem("ez-ipupdate -h %s -s ns1.asuscomm.com -S %s -i %s -A 1", host, service, wan_ifname);
	}
	else
#endif //End of Yau add

	if (strlen(service) > 0)
	{
//		dprintf("ddns update %s %s\n", server, service);
		dbG("ddns update %s %s\n", server, service);

		nvram_unset("ddns_cache");
		nvram_unset("ddns_ipaddr");
		nvram_unset("ddns_status");
//		nvram_set("ddns_updated", "1");

		if (pids("ez-ipupdate"))
		{
			system("killall -SIGINT ez-ipupdate");
			sleep(1);
		}

		doSystem("ez-ipupdate -S %s -i %s -u %s -h %s -e /sbin/ddns_updated -b /tmp/ddns.cache %s", service, wan_ifname, usrstr, host, wild ? "-w" : "");
	}

	return 0;
}

void
stop_ddns(void)
{
	int delay_count = 10;

	if (pids("ez-ipupdate"))
		system("killall -SIGINT ez-ipupdate");
	else
		return;

	while (pids("ez-ipupdate") && (delay_count-- > 0))
		sleep(1);
}

void
stop_syslogd()
{
	if (pids("syslogd"))
		system("killall syslogd");
}

void
stop_klogd()
{
	if (pids("klogd"))
		system("killall klogd");
}

int 
start_syslogd()
{
	pid_t pid;

	time_zone_x_mapping();
	setenv("TZ", nvram_safe_get("time_zone_x"), 1);

	if (!nvram_match("log_ipaddr", ""))
	{
		char *syslogd_argv[] = {"/sbin/syslogd", "-m", "0", "-t", nvram_safe_get("time_zone_x"), "-O", "/tmp/syslog.log", "-R", nvram_safe_get("log_ipaddr"), "-L", NULL};

		_eval(syslogd_argv, NULL, 0, &pid);
	}
	else
	{
		char *syslogd_argv[] = {"/sbin/syslogd", "-m", "0", "-t", nvram_safe_get("time_zone_x"), "-O", "/tmp/syslog.log", NULL};

		_eval(syslogd_argv, NULL, 0, &pid);
	}
}

int
start_klogd()
{
	pid_t pid;

	if (!nvram_match("log_ipaddr", ""))
	{
#ifdef KERNEL_DBG
		char *klogd_argv[] = {"/sbin/klogd", "-d", NULL};
#else
		char *klogd_argv[] = {"/sbin/klogd", NULL};
#endif
		_eval(klogd_argv, NULL, 0, &pid);	// 1003 disable
	}
	else
	{
#ifdef KERNEL_DBG
		char *klogd_argv[] = {"/sbin/klogd", "-d", NULL};
#else
		char *klogd_argv[] = {"/sbin/klogd", NULL};
#endif
		_eval(klogd_argv, NULL, 0, &pid);	// 1003 disable
	}
}

int 
start_logger(void)
{		
	start_syslogd();
	start_klogd();

	return 0;
}

int 
start_misc(void)
{ 
	char *infosvr_argv[] = {"infosvr", "br0", NULL};
	char *watchdog_argv[] = {"watchdog", NULL};
	pid_t pid;

	_eval(infosvr_argv, NULL, 0, &pid);
	_eval(watchdog_argv, NULL, 0, &pid);

	return 0;
}

void
stop_misc(void)
{
	dbg("stop_misc()\n");

	if (pids("infosvr"))
		system("killall infosvr");
	if (pids("watchdog"))
		system("killall watchdog");
	if (pids("ntp"))
		system("killall -SIGTERM ntp");
	if (pids("ntpclient"))
		system("killall ntpclient");
	if (pids("pspfix"))
		system("killall pspfix");
	stop_wsc();
	stop_wsc_2g();
	stop_lltd();
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	if (pids("detectWan"))
		system("killall detectWan");
	stop_rstats();
	kill_pidfile_s("/var/run/detect_internet.pid", SIGTERM);
#endif
	if (pids("tcpcheck"))
		system("killall -SIGTERM tcpcheck");
	if (pids("traceroute"))
		system("killall traceroute");
	if (pids("usbled"))
		system("killall -SIGTERM usbled");
}

void
stop_misc_no_watchdog(void)
{
	dbg("stop_misc_no_watchdog()\n");

	if (pids("infosvr"))
		system("killall infosvr");
	if (pids("ntp"))
		system("killall -SIGTERM ntp");
	if (pids("ntpclient"))
		system("killall ntpclient");
	if (pids("udhcpc"))
		system("killall -SIGTERM udhcpc");
	if (pids("pspfix"))
		system("killall pspfix");
	stop_wsc();
	stop_wsc_2g();
	stop_lltd();	// 1017 add
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	if (pids("detectWan"))
		system("killall detectWan");
	stop_rstats();
	kill_pidfile_s("/var/run/detect_internet.pid", SIGTERM);
#endif
	if (pids("tcpcheck"))
		system("killall -SIGTERM tcpcheck");
	if (pids("traceroute"))
		system("killall traceroute");
	if (pids("usbled"))
		system("killall -SIGTERM usbled");
	dprintf("done\n");
}


#ifndef USB_SUPPORT
#else

void
start_u2ec()
{
#ifdef U2EC
//	start_usdsvr_broadcast();
//	start_usdsvr_unicast();
	if (!pids("u2ec"))
	{
		unlink("/var/run/u2ec.pid");
		system("u2ec &");

		nvram_set("apps_u2ec_ex", "1");
	}
#endif
}

void
stop_u2ec()
{
//	system("killall usdsvr_broadcast");
//	system("killall usdsvr_unicast");
	int delay_count = 10;

	if (pids("u2ec"))
		system("killall -SIGKILL u2ec");
	else
		return;

	while (pids("u2ec") && (delay_count-- > 0))
		sleep(1);
}

extern char usb_path1[];
extern char usb_path1_old[];
extern char usb_path2[];
extern char usb_path2_old[];

int
start_usb(void)
{
	nvram_set("usb_path", "");
	nvram_set("usb_path1", "");
	nvram_set("usb_path2", "");

	system("insmod -q usblp");
	system("insmod -q usb-storage");
/*
#ifdef CDMA
	system("insmod acm.o");
	system("insmod usbserial.o vendor=0x1165 product=0x0001");
#endif
*/
	start_u2ec();
	start_lpd();
}

void
stop_usb(void)
{
	dbg("%s()...\n", __FUNCTION__);

#ifdef U2EC
	stop_u2ec();
	stop_lpd();
#endif
//	if (!strcmp(usb_path1, "storage"))
	if (!strcmp(nvram_safe_get("usb_path1"), "storage"))
		remove_usb_mass("1");
//	if (!strcmp(usb_path2, "storage"))
	if (!strcmp(nvram_safe_get("usb_path2"), "storage"))
		remove_usb_mass("2");
}

#ifdef DLM
void write_ftpd_conf()
{
	FILE *fp;
	char user[64], user1[64], password[64], path[64];
	char tmpstr[64];
	char rright[384], wright[384], maxuser[16];
	int snum, unum, i, j;
	char test_path[64];
	char *tmp1=NULL;
	char root_path[64];

	/* write /etc/vsftpd.conf */
	fp=fopen("/tmp/vsftpd.conf", "w");
	if (fp==NULL) return;

	if (nvram_match("st_ftp_mode", "2"))
		fprintf(fp, "anonymous_enable=NO\n");
	else{
		fprintf(fp, "anonymous_enable=YES\n");
		fprintf(fp, "anon_upload_enable=YES\n");
		fprintf(fp, "anon_mkdir_write_enable=YES\n");
		fprintf(fp, "anon_other_write_enable=YES\n");
	}

	fprintf(fp, "nopriv_user=root\n");
	fprintf(fp, "write_enable=YES\n");
	fprintf(fp, "local_enable=YES\n");
	fprintf(fp, "chroot_local_user=YES\n");
	fprintf(fp, "local_umask=000\n");
	fprintf(fp, "dirmessage_enable=NO\n");
	fprintf(fp, "xferlog_enable=NO\n");
	fprintf(fp, "syslog_enable=NO\n");
	fprintf(fp, "connect_from_port_20=YES\n");
	fprintf(fp, "listen=YES\n");
	fprintf(fp, "pasv_enable=YES\n");
	fprintf(fp, "ssl_enable=NO\n");
	fprintf(fp, "tcp_wrappers=NO\n");
	strcpy(maxuser, nvram_safe_get("st_max_user"));
	if ((atoi(maxuser)) > 0)
		fprintf(fp, "max_clients=%s\n", maxuser);
	else
		fprintf(fp, "max_clients=%s\n", "10");
	fprintf(fp, "ftp_username=anonymous\n");
	fprintf(fp, "ftpd_banner=Welcome to ASUS %s FTP service.\n", nvram_safe_get("productid"));

	if (strcmp(nvram_safe_get("ftp_lang"), "EN") != 0)
	{
		fprintf(fp, "enable_iconv=YES\n");
		if (nvram_match("ftp_lang", "TW")) {
			fprintf(fp, "remote_charset=cp950\n");
		}
		else if (nvram_match("ftp_lang", "CN")) {
			fprintf(fp, "remote_charset=cp936\n");
		}
	}

	fclose(fp);
}

void
start_ftp()
{
	if (nvram_match("enable_ftp", "0")) return;

	stop_ftp();

	write_ftpd_conf();

	if (nvram_match("st_ftp_mode", "1"))
		dbg("ftp mode: login to first partition\n");
	else if (nvram_match("st_ftp_mode", "2"))
		dbg("ftp mode: login to first matched shared node\n");

	system("vsftpd&");

	if (pids("vsftpd"))
		logmessage("FTP server", "daemon is started");
}

int
test_user(char *target, char *pattern)
{
	char s[384];
	char p[32];
	char *start;
	char *pp;
	strcpy(s, target);
	strcpy(p, pattern);
	start = s;
	while (pp=strchr(start, ';'))
	{
		*pp='\0';
		if (! strcmp(start, p))
			return 1;
		start=pp+1;
	}
	return 0;
}
#endif

#ifdef USBTPT
int
usbtpt(int argc, char *argv[])
{
	struct timeval tv1, tv2;
	struct timezone tz1, tz2;
	float diff = 0.0, rate = 0.0;
	int fd, Mbsize = 0, len;
	int i, counts;

	if (argc < 4)
		return 0;

	if (strcmp("argv[1]", "-h") == 0)
	{
		dbg("usage: utpt [file] [size(Mb)] [buflen(Kb)]\n");
		return 0;
	}

	len = atoi(argv[3])*1024;
	char buf[len];

	Mbsize = atoi(argv[2]) * 1024 * 1024;
	counts = Mbsize / len;

	memset(buf, 'a', sizeof(buf));
	unlink(argv[1]);

	dbg("write size is %d, buf len is %d, counts is %d\n", Mbsize, len, counts);

	gettimeofday(&tv1, &tz1);

	if ((fd = open(argv[1], O_CREAT|O_WRONLY)) < 0)
	{
		perror("open");
		exit(-1);
	}
	for (i=0; i < counts; ++i)
		write(fd, buf, sizeof(buf));

	close(fd);

	gettimeofday(&tv2, &tz2);


	diff = (float)((float)(tv2.tv_sec - tv1.tv_sec) + ((float)(tv2.tv_usec - tv1.tv_usec))/1000000);
	rate = (float)((float)Mbsize/diff)/(float)(1024*1024);
	dbg("tv1 = (%d, %d), tv2 = (%d, %d), diff is %.6f, rate is %.3f Mbps\n", tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec, diff, rate);
	char tmpstr[40];
	sprintf(tmpstr, "chmod 666 %s", argv[1]);
	system(tmpstr);
	return 0;
}
#endif

/* remove usb mass storage */
int
//remove_usb_mass(char *product)
remove_usb_mass(char *port)
{
	FILE *fp;
	int is_apps_running_when_umount = 0;
	dbg("is_apps_running_when_umount: %d\n", is_apps_running_when_umount);

	if (!port)
	{
		dbg("%s() umount ejected only\n", __FUNCTION__);
		goto do_umount;
	}
	else
		dbg("%s() port: %s\n", __FUNCTION__, port);

#ifdef REMOVE
	if (product!=NULL)
		logmessage("USB storage", product);
	else
		logmessage("USB storage", "NULL");
#endif
#ifndef NO_DM
	int delay_for_DM = 0;
	is_apps_running_when_umount = is_apps_running();
	dbg("is_apps_running_when_umount: %d\n", is_apps_running_when_umount);
	if (is_apps_running_when_umount)
	{
		delay_for_DM = 1;
		nvram_set("dm_block", "1");

		if (pids("snarf"))
			system("killall -SIGKILL snarf");
		if (pids("giftd"))
			system("killall -SIGKILL giftd");
		if (pids("rtorrent"))
			system("killall -SIGKILL rtorrent");
		if (pids("dmathined"))
			system("killall -SIGKILL dmathined");
	}
#endif
	stop_samba();
	stop_ftp();
	stop_dms();
#ifdef CONFIG_USER_MTDAAPD
	stop_mt_daapd();
#endif
	if (pids("usbtest"))
		system("killall -SIGKILL usbtest");
	if (delay_for_DM)
	{
		dbg("sleep 10 seconds for DM termination\n");
		sleep(10);
	}

do_umount:
	umount_usb_path(port);
	umount_ejected();
	chk_partitions(USB_PLUG_ON);

	dbg("is_apps_running_when_umount: %d\n", is_apps_running_when_umount);
	if (is_apps_running_when_umount)
	{
		if (nvram_match("wan_route_x", "IP_Routed"))
		{
			dbg("rc_restart_firewall...\n");
			rc_restart_firewall();
		}
#if 0
		if ((fp=fopen("/proc/sys/net/nf_conntrack_max", "w+")))
		{
			if (nvram_get("misc_conntrack_x") == NULL)
				fputs(MAX_CONNTRACK_DM, fp);
			else
			{
				dbg("\nrestore nf_conntract_max...\n\n");
				fputs(nvram_safe_get("misc_conntrack_x"), fp);
			}

			fclose(fp);
		}
#endif
	}

	dbg("You can plugoff usb now\n");
	return 0;
}

int
remove_usb_3g()
{
	dbg("## remove usb 3g dev\n");	// tmp test
	if (pids("pppd"))
		system("killall -SIGKILL pppd");
	system("rmmod usbserial");
	system("rmmod hso");
	nvram_set("wan0_ipaddr", "");
	return 0;
}

#if 0
int
remove_storage_main(void)
{
	remove_usb_mass(NULL);
	return 0;
}
#endif

#define MOUNT_VAL_FAIL 	0
#define MOUNT_VAL_RONLY	1
#define MOUNT_VAL_RW 	2

#ifdef DLM
int calc_crc32( const char *fname, unsigned long *crc ) {
    FILE *in;		/* input file */
    unsigned char buf[BUFSIZ]; /* pointer to the input buffer */
    size_t i, j;	/* buffer positions*/
    int k;		/* generic integer */
    unsigned long tmpcrc=0xFFFFFFFF;

    /* open file */
    if ((in = fopen(fname, "rb")) == NULL) return -1;

    /* loop through the file and calculate CRC */
    while ( (i=fread(buf, 1, BUFSIZ, in)) != 0 ) {
	for (j=0; j<i; j++) {
	    k=(tmpcrc ^ buf[j]) & 0x000000FFL;
	    tmpcrc=((tmpcrc >> 8) & 0x00FFFFFFL) ^ crcs[k];
	}
    }
    fclose(in);
    *crc=~tmpcrc; /* postconditioning */
    return 0;
}

#ifndef NO_DM
int
ckeck_apps_completeness(const char *dirname)
{
	FILE *fp=NULL;
	FILE *fp2=NULL;
	char line[256];
	char line2[256];
	//char listfilename[64]="/apps/list";
	//char crcfilename[64]="/apps/crc";
	char listfilename[32]="/tmp/harddisk/part0/.apps/list";
	char crcfilename[32]="/tmp/harddisk/part0/.apps/crc";
	char appsdir[64];
	char tmp[256];
	char crc_str[9];
	unsigned long crc;
	unsigned long crc_org;
	
	dbg("chk apps completness\n");	// tmp test
	sprintf(appsdir, "%s%s", dirname, "/.apps");

	//dbg("check apps completeness...ing\n");	// tmp test
	if ((fp=fopen(listfilename, "r"))==NULL)
	{
		dbg("Cannot find %s.\n", listfilename);
		dbg("Make sure it's available.\n");
		return 0;
	}
	
	if ((fp2=fopen(crcfilename, "r"))==NULL)
	{
		dbg("Cannot find %s.\n", crcfilename);
		dbg("Make sure it's available.\n");
		fclose(fp);
		return 0;
	}

	dbg("chk comp 1\n");	// tmp test
	while (!feof(fp))
	{
		if (fgets(line,sizeof(line),fp)==NULL) break;	/* end-of-file */
		if (strlen(line)<3) continue;			/* line empty */
		if ((strncmp(line, "./crc", 5) == 0) || (strncmp(line, "./list", 6) == 0))
			continue;
		
		sprintf(tmp, "%s%s", appsdir, line+1);		/* file path */
		tmp[strlen(tmp)-1]='\0';

		dbg("check crc [%s]\n", tmp);	// tmp test
		if (calc_crc32(tmp, &crc) != 0)
		{
			dbg("Error reading file %s.\n", tmp);
			sprintf(tmp, "rm -rf %s", appsdir);
			system(tmp);	
			fclose(fp);
			fclose(fp2);
			return 0;
		}
		
		if (fgets(line2,sizeof(line2),fp2)==NULL)
		{
			dbg("fgets err\n");	// tmp test
			sprintf(tmp, "rm -rf %s", appsdir);
			system(tmp);
			fclose(fp);
			fclose(fp2);
			return 0;
		}
		sprintf(crc_str, "%08lX", crc);
		dbg("CRC32 now: %s\n", crc_str);	// tmp test

		line2[strlen(line2)-1]='\0';
		dbg("CRC32 org: %s\n", line2);
		
		if (strcmp(crc_str, line2)!=0)
		{
			dbg("compare crc err\n");	// tmp test
			sprintf(tmp, "rm -rf %s", appsdir);
			system(tmp);
			fclose(fp);
			fclose(fp2);
			return 0;
		}
	}

	fclose(fp);
	fclose(fp2);
	return 1;
}
#endif	// NO_DM

#endif	// #ifdef DLM

unsigned long file_size(const char *filepath);

int
write_file(const char *swap_path, int buf_size, int runs, int index)
{
//	dbG("file: %s, buf size: %d, runs: %d, index: %d\n", swap_path, buf_size, runs, index);

	FILE *fp;
	char write_buf[buf_size];
	int i;
	//int count = 0;
	//unsigned long sz, sz_old;

	memset(write_buf, 0, buf_size);

	fp = fopen(swap_path, "a");
	if (fp!=NULL)
	{
		//sz = file_size(swap_path);
		dbg("--(cric) start to write swap file[%d]:(runs:%d)--\n", index, runs);	// tmp test
		for (i = 0; i< runs; i++)
		{
			fwrite(write_buf, buf_size, 1, fp);
			//count++;
			//sz_old = sz;
			//sz = file_size(swap_path);
//			if (sz_old != 0)
//				dbG("file size diff: %d ", (sz - sz_old));
//			dbG("file size after %d write: %d\n", count, sz);
		}

//		dbG("write count: %d\n", count);
/*
		if (sz % (1024*1024))
		{
			fclose(fp);
			if (fp = fopen(swap_path, "a"))
			{
				fwrite(write_buf, buf_size, 1, fp);
				fclose(fp);
			}
		}
		else
			fclose(fp);
*/
	}
	else
	{
		perror("fopen /media/AiDisk_xx/.swap");
		return -1;
	}

	fclose(fp);
	dbg("--(cric) end to write swap file[%d]--\n", index);  // tmp test
	return 0;
}

int swap_write_count = 0;

int 
create_swap_file(const char *swap_path)
{
	unsigned long total_swap_size = 1024*1024*128;
	unsigned long unit_size = 1024*256;
	unsigned long wr_num = 8;
	unsigned long truncate_run = total_swap_size / unit_size / wr_num;
	char tmpstr[128];
	int result = 0, i;
	time_t now, elapsed;
	time_t start_time = uptime();
	time_t timeout = start_time + 60;

	if (!nvram_match("asus_mfg", "0"))
		return -1;

	if (swap_check())
	{
		printf("swap already exist\n");	// tmp test
		return -1;
	}

	if (!swap_path)
		return -1;
	else
		doSystem("rm -f %s", swap_path);

//	if (check_if_file_exist(swap_path))
//		dbG("%s exists!\n", swap_path);
//	else
//		dbG("%s not found\n", swap_path);

	if (nvram_match("mnt_type", "ntfs")) timeout = timeout + 30;
	swap_write_count = 0;

	for (i = 0; i < wr_num; ++i)
	{
		if (i) usleep(200000);
		if ((result = write_file(swap_path, unit_size, truncate_run, i)) < 0)
			break;

//		dbG("swap file size: %d\n", file_size(swap_path));

		now = uptime();
		elapsed = now - start_time;
		dbg("# elapsed %d seconds\n", elapsed);
		if (elapsed >= timeout)
		{	// stop create & stop service
			dbg("slow usb storage writing speed, stop swap creating\n");
			logmessage("USB storage", "slow usb storage writing speed, stop swap creating.");
			break;
		}

		/* workaround for buggy ntfs module */
		if (file_size(swap_path) > total_swap_size)
		{
			printf("ufsd write error! exceed totoal swap size\n");	// tmp test
			break;
		}
	}
	swap_write_count = i;
	dbg("swap write count is %d\n", swap_write_count);

//	if (result == 0)
	if (file_size(swap_path) >= total_swap_size)
	{
		memset(tmpstr, 0, sizeof(tmpstr));
		sprintf(tmpstr, "mkswap %s", swap_path);
		system(tmpstr);

		memset(tmpstr, 0, sizeof(tmpstr));
		sprintf(tmpstr, "swapon %s", swap_path);
		system(tmpstr);

		if (swap_check())
		logmessage("USB storage", "%dk swap file is added", swap_check() / 1024);
	}	

	return result;
}

/* insert usb mass storage */
int
hotplug_usb_mass(char *product)
{	
	int n = 0, m = 0, p = 0;
	struct dirent *dp, *dp_disc, **dpopen;
	char swap_path[128], test_path[128];
	int i, j;
	int chk_freeDisk;
	int apps_comp = 0;
	int apps_status = 0;
	int apps_disk_free = 0;
	int new_disc[2];
	new_disc[0] = 0;
	new_disc[1] = 0;
# ifdef DLM
	int buflen=0;
# endif
	int skip_DM = 1;

	char *usb_mass_first_path = nvram_safe_get("usb_mnt_first_path");
	if (!strlen(usb_mass_first_path))
		usb_mass_first_path = "fail_path";
	dbg("\n[hotplug] get usb mass first path = %s\n", usb_mass_first_path);

//	nvram_set("usb_disc_mount_path", usb_mass_first_path);
//	nvram_set("usb_disc_fs_path", usb_mass_first_path);
//	nvram_set("usb_disc_path", "/tmp/harddisk/part0");

	if (!check_if_dir_exist("/tmp/harddisk/part0"))
	{
		dbg("\n[hotplug] link part0 to the first usb path (%s)\n", usb_mass_first_path);

		for (i = 0; i < 3; ++i)
		{
			if (symlink(usb_mass_first_path, "/tmp/harddisk/part0") == 0)
				break;
			else
			{
				system("rm -Rf /tmp/harddisk/part0");	// cancel the orig link, won't destroy the orig content
				dbg("[hotplug] link to part0 fail, retrying...(%d)\n", i);
				sleep(1);
			}
		}
	}

	//printf("chk link now...\n");
	//sleep(20);
//	nvram_set("usb_disc0_path0", "/tmp/harddisk/part0");
	
#ifdef DLM
	memset(test_path, 0, sizeof(test_path));
	strcpy(test_path, "/tmp/harddisk/part0");

#ifndef NO_DM
	if (nvram_match("asus_mfg", "0") && !is_apps_running())
	{
		if ((apps_comp=ckeck_apps_completeness(test_path))==1)
			nvram_set("apps_comp", "1");
		else
		{
			dbg("verify apps fail\n");	// tmp test
			system("rm -Rf /tmp/harddisk/part0/.apps");

			nvram_set("apps_comp", "0");
			nvram_set("dm_block", "0");	// 0716 chk
		}
	}
#else
	nvram_set("apps_comp", "1");
#endif	// #ifndef NO_DM

#ifndef NO_DM
	dbg("\n\n**apps_completeness = %d\n\n", apps_comp);	// tmp test

	/* chk disk total > 1G */
	if (nvram_match("asus_mfg", "0") && !is_apps_running())
	{
		nvram_set("apps_disk_free", "0");

		chk_freeDisk = check_disk_free_GE_1G(test_path);			
		dbg("chk freeDisk result= %d\n", chk_freeDisk);	// tmp test

		if ((chk_freeDisk == 2)/* || (chk_freeDisk == 3)*/)
		{
			nvram_set("apps_disk_free", "1");

			if (nvram_match("apps_dl", "1"))
				skip_DM = 0;
		}
		else if (chk_freeDisk == 1)
			logmessage("USB storage", "The swap file is not added for free space is less than 128 Mb");
		else if (chk_freeDisk == 3)	/* accept to create swap */
			logmessage("USB storage", "The swap file is not added for partition size is less than 1 G");
		else if (chk_freeDisk == 0)
			logmessage("USB storage", "The swap file is not added for unknown reasons");
	}
	else
	{
		logmessage("USB storage", "ATE mode? DM is already running?");
	}
#else
	check_disk_free_GE_1G(test_path);
#endif	// #ifndef NO_DM

	/* make necessary dir */
#ifndef NO_DM
	if (!skip_DM)
		init_apps();
#endif

	/* run ftp latter*/
//	doSystem("/sbin/test_of_var_files %s", usb_mass_first_path);	// move to busybox mdev.c

#ifdef CONFIG_USER_MTDAAPD
	if (nvram_match("asus_mfg", "0") && nvram_match("wan_route_x", "IP_Routed") && !pids("mDNSResponder"))
	{
		if (!nvram_match("computer_name", "") && is_valid_hostname(nvram_safe_get("computer_name")))
			doSystem("mDNSResponder %s thehost %s _daap._tcp. 3689 &", nvram_safe_get("lan_ipaddr_t"), nvram_safe_get("computer_name"));
		else
			doSystem("mDNSResponder %s thehost %s _daap._tcp. 3689 &", nvram_safe_get("lan_ipaddr_t"), nvram_safe_get("productid"));
	}
#endif
	if (nvram_match("asus_mfg", "0"))
		run_dms();

	run_ftpsamba();
#ifdef CONFIG_USER_MTDAAPD
	if (nvram_match("asus_mfg", "0"))
		start_mt_daapd();
#endif
	/* chk disk free */
#ifndef NO_DM
	if (!skip_DM)
		apps_disk_free = check_disk_free_apps(test_path, apps_comp);
#else
	nvram_set("apps_disk_free", "1");
#endif	// #ifndef NO_DM

	//dbg("\nchk apps ready: apps_comp=%d, apps_dl=%s, apps_running=%d, chk_freeDisk=%d, swap_write_count=%d\n", apps_comp, nvram_safe_get("apps_dl"), is_apps_running(), chk_freeDisk, swap_write_count);	// tmp test
	//dbg("it supposed to be 1, 1, 0, 2, 6");	// tmp test
	/* if apps is ready and not running, then run it */
#ifndef NO_DM
	if (!skip_DM)
	{
		sprintf(swap_path, "%s/.swap", test_path);
		eval("rm", "-f", swap_path);	// previous swap file may have wierd run time error

		if (nvram_match("apps_dl", "0"))
		{
			dbg("skip running DM: apps disabled\n");
			logmessage("Download Master", "apps disabled, daemon is not started");
		}
		else if ((apps_comp == 1) && nvram_match("apps_dl", "1") && !is_apps_running() &&
			 //(swap_check() || create_swap_file(swap_path) >= 0))
			 ( create_swap_file(swap_path) >= 0))
		{
			if ((chk_freeDisk == 2) && (swap_write_count >= 3) && (!nvram_match("dm_block", "1")))
			{
				if (apps_comp==1 && apps_disk_free==1)
				{
					if (nvram_get("dm_delay") && atoi(nvram_get("dm_delay")))
					{
						dbg("dm_delay: %s second(s)\n", nvram_get("dm_delay"));
						int dm_delay = atoi(nvram_get("dm_delay"));

						while (dm_delay > 0)
						{
							dbg(".");
							sleep(1);
							dm_delay--;
						}
						dbg("\n");
					}
					else
					{
						dbg("invalid dm_delay value\n");
						sleep(6);
					}

					exec_apps();
				}
			}
			else
			{
				dbg("skip running DM: not enough disk space or slow disk\n");
				logmessage("Download Master", "not enough space or slow disk, daemon is not started");
			}
		}
		else
		{
			dbg("skip running DM: apps not ready\n");
			logmessage("Download Master", "apps not ready, daemon is not started");
		}
	}
#endif

#endif	// #ifdef DLM

	return 0;
}

double free_max;

int
chk_free(char *diskpath)
{
	struct statfs fsbuf;
	double free_size, block_size;

	if (statfs(diskpath, &fsbuf))
	{
		perror("*** check_disk_free: statfs fail!");
		return 0;
	}

	free_size = (double)((double)((double)fsbuf.f_bfree * fsbuf.f_bsize)/(1024*1024));
	dbg("\n[compete free] chk free: free=%fMB\n", free_size);	// tmp test

	if ( free_size > free_max )
	{
		free_max = free_size;
		return 1;
	}
	else
		return 0;
}

int
compete_choose(int disk_num, int part_num)
{
	char mnt_path[32];

	sprintf(mnt_path, "/media/AiDisk_%c%d", disk_num+97, part_num);
	return chk_free(mnt_path);
}

void
mnt_op(int disk_num, int part_num, int op_mode)
{
	char dev_path[32], mnt_path[32], cmd[128];

	memset(mnt_path, 0, sizeof(mnt_path));
	memset(dev_path, 0, sizeof(dev_path));
	memset(cmd, 0, sizeof(cmd));

	switch(op_mode) {
/*
	case OP_MOUNT:	// only needs for disk w/ no partition table
		if (part_num < 2)
			sprintf(dev_path, "sd%c", disk_num+97);
		else
			sprintf(dev_path, "sd%c%d", disk_num+97, part_num);
		sprintf(mnt_path, "AiDisk_%c%d", disk_num+97, part_num);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/sbin/automount.sh %s %s", dev_path, mnt_path);
		system(cmd);
		dbg("\n##[mnt_op]: MOUNT: %s(%d,%d)\n", cmd, disk_num, part_num);	// tmp test
		break;
	case OP_UMOUNT:
		sprintf(mnt_path, "/media/AiDisk_%c%d", disk_num+97, part_num);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "swapoff %s/.swap", mnt_path);
		dbg("### [mnt_op] ready to %s\n", cmd);	// tmp test
		system(cmd);
		sleep(1);

		dbg("### [mnt_op] ready to remove .swap\n");	// tmp test
		system("rm -f /tmp/harddisk/part0/.swap");

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "umount2 %s", mnt_path);
		system(cmd);
		dbg("### [mnt_op]: UMOUNT: %s\n", cmd);	// tmp test
		break;
*/
	case OP_SETNVRAM:
		sprintf(mnt_path, "/media/AiDisk_%c%d", disk_num+97, part_num);

		dbg("[mnt_op]: set usb_mnt_first_path as %s\n", mnt_path);	// tmp test

		if (!strlen(nvram_safe_get("usb_mnt_first_path")) && !found_mountpoint_with_apps())
		{
			dbg("\n\n\nmnt path: %s, path1 dev: %s, path2 dev: %s, find...: %s %d %d\n\n\n", mnt_path, nvram_safe_get("usb_path1_act"), nvram_safe_get("usb_path2_act"), find_sddev_by_mountpoint(mnt_path), nvram_match("usb_path1_act", find_sddev_by_mountpoint(mnt_path)), nvram_match("usb_path2_act", find_sddev_by_mountpoint(mnt_path)));

			nvram_set("usb_mnt_first_path", mnt_path);
			if (!strncmp(nvram_safe_get("usb_path1_act"), find_sddev_by_mountpoint(mnt_path), 3))
				nvram_set("usb_mnt_first_path_port", "1");
			else if (!strncmp(nvram_safe_get("usb_path2_act"), find_sddev_by_mountpoint(mnt_path), 3))
				nvram_set("usb_mnt_first_path_port", "2");
			else
				nvram_set("usb_mnt_first_path_port", "0");

			nvram_set("apps_status_checked", "1");	// it means need to check
			nvram_set("apps_comp", "0");
			nvram_set("apps_disk_free", "0");
		}	
		break;

	default:
		break;
	}
}

/*
int test_write_result;

int 
test_write_firstP()
{
	int fd;
	char *first_mnt_path, test_file[128];

	memset(test_file, 0, sizeof(test_file));
	first_mnt_path = nvram_safe_get("usb_mnt_first_path");
	sprintf(test_file, "%s/test_write_file", first_mnt_path);

	if ((fd = open(test_file, O_CREAT|O_RDWR)) <= 0)
	{
		dbg("[failed open...test write before start apps\n]");
		return -1;
	}
	if (write(fd, "test write", 10) <= 0)
	{
		dbg("[failed write...test write before start apps\n]");
		return -1;
	}
	close(fd);
	unlink(test_file);
	return 1;
}
*/

#if 0
int 
chk_safe_fs(int chk_bit, char fs_type)
{
/*
	if (chk_bit)
	{
		if (fs_type=='n')
			return 0;
		else
			return 1;
	}
	else
*/
	return 1;
}
#endif

void 
set_mnt_type(char fs_type)
{
	if (fs_type == 'n')
		nvram_set("mnt_type", "ntfs");
	else if (fs_type == 'f')
		nvram_set("mnt_type", "fat32");
	else if (fs_type == '2')
		nvram_set("mnt_type", "ext2");
	else if (fs_type == '3')
		nvram_set("mnt_type", "ext3");
	else
		nvram_set("mnt_type", "unknown");
}

void 
pre_chk_mount()
{
	FILE *fp_p = fopen("/proc/partitions", "r");
	FILE *fp_m = fopen("/proc/mounts", "r");
	char buf[128];
	char par_n[26][20], mnt_n[26][20];
	char pc, pcn[3], *tp;
	int  pd;
	int  i, j, k;
	int  try_counts = 0;
//	int  set_nv = 0;

	while ((!fp_m) || (!fp_p))
	{
		if (try_counts > 10)
		{
			dbg("cannot open mounts/partitions\n");
			return;
		}

		if (!fp_m)
			fp_m = fopen("/proc/mounts", "r");
		if (!fp_p)
			fp_p = fopen("/proc/partitions", "r");

		++try_counts;
		dbg("open mounts/part fail, try [%d]\n", try_counts);
		sleep(1);
	}

	memset(par_n, 0, sizeof(par_n));
	memset(buf, 0, sizeof(buf));
	while (fgets(buf, sizeof(buf), fp_p))	// chk partitions
	{
		if (strstr(buf, "mtdblock"))
			continue;
		if ((tp = strstr(buf, "sd")) != NULL)
		{
			pc = *(tp+2);
			memset(pcn, 0, sizeof(pcn));
			if ((*(tp+3)) && ((*(tp+3)) != '\n') && ((*(tp+3)) != '\r') && ((*(tp+3)) != '\0'))
				pcn[0] = *(tp+3);
			if ((*(tp+4)) && ((*(tp+4)) != '\n') && ((*(tp+4)) != '\r') && ((*(tp+4)) != '\0'))
				pcn[1] = *(tp+4);
			pd = atoi(pcn);
			if (pc >= 97)	// 'a' is 97
			{
				par_n[pc-97][pd] = 'h';
			}
		}
		memset(buf, 0, sizeof(buf));
	}

	memset(mnt_n, 0, sizeof(mnt_n));
	memset(buf, 0, sizeof(buf));
	while (fgets(buf, sizeof(buf), fp_m))    // chk mounts
	{
		if (!strstr(buf, "/media/AiDisk_"))
			continue;
		if ((tp = strstr(buf, "/media/AiDisk_")) != NULL)
		{
			pc = *(tp+14);
			memset(pcn, 0, sizeof(pcn));
			if ((*(tp+15)) && ((*(tp+15)) != ' '))
				pcn[0] = *(tp+15);
			if ((*(tp+16)) && ((*(tp+16)) != ' '))
				pcn[1] = *(tp+16);
			pd = atoi(pcn);
			if (pc >= 97)    // 'a' is 97, chk if starts from 'a'... and write flag 'h'(has)
			{
				mnt_n[pc-97][pd] = 'h';
			}
		}
		memset(buf, 0, sizeof(buf));
	}

	fclose(fp_p);
	fclose(fp_m);
/*
	for (i=0; i<26; ++i)     // check disk w/ no partition table and mount it
	{
		if ((par_n[i][0] == 'h') && (par_n[i][1] != 'h') && (mnt_n[i][1] != 'h'))
		{
			mnt_op(i, 1, OP_MOUNT);
		}
		for (j=2; j<20; ++j)     // no need to chk sdx0/1
		{
			if ((par_n[i][j] == 'h') && (mnt_n[i][j] != 'h'))
				mnt_op(i, j, OP_MOUNT);
		}
	}
*/
}

void 
chk_partitions(int state)
{
	FILE *fp_p = fopen("/proc/partitions", "r");
	FILE *fp_m = fopen("/proc/mounts", "r");
	char buf[128];
	char par_n[26][20], mnt_n[26][20], fs_n[26][20];
	char pc, pcn[3], *tp;
	int  pd;
	int  i, j, k;
	int  try_counts = 0;
//	int  set_nv = 0;
	int  new_mount = -1;
	int  need_choose_fs = 0;
	int  choose_case = 0, op_i = -1, op_j = -1;

	dbg("chk_partition tables\n");	// tmp test
	while ((!fp_m) || (!fp_p))
	{
		if (try_counts > 10)
		{
			dbg("cannot open mounts/partitions\n");
			return;
		}

		if (!fp_m)
			fp_m = fopen("/proc/mounts", "r");
		if (!fp_p)
			fp_p = fopen("/proc/partitions", "r");

		++try_counts;
		dbg("open mounts/part fail, try [%d]\n", try_counts);
		sleep(1);
	}

	memset(par_n, 0, sizeof(par_n));
	memset(buf, 0, sizeof(buf));
	while (fgets(buf, sizeof(buf), fp_p))	// chk partitions
	{
		if (strstr(buf, "mtdblock"))
			continue;
		if ((tp = strstr(buf, "sd")) != NULL)
		{
			pc = *(tp+2);
			memset(pcn, 0, sizeof(pcn));
			if ((*(tp+3)) && ((*(tp+3)) != '\n') && ((*(tp+3)) != '\r') && ((*(tp+3)) != '\0'))
				pcn[0] = *(tp+3);
			if ((*(tp+4)) && ((*(tp+4)) != '\n') && ((*(tp+4)) != '\r') && ((*(tp+4)) != '\0'))
				pcn[1] = *(tp+4);
			pd = atoi(pcn);
			if (pc >= 97)	// 'a' is 97
			{
				par_n[pc-97][pd] = 'h';
				//dbg("**set part[%d][%d] **\n", pc-97, pd);	// tmp test
			}
		}
		memset(buf, 0, sizeof(buf));
	}

	memset(mnt_n, 0, sizeof(mnt_n));
	memset(fs_n, 0, sizeof(fs_n));
	memset(buf, 0, sizeof(buf));
	while (fgets(buf, sizeof(buf), fp_m))    // chk mounts
	{
		if (!strstr(buf, "/media/AiDisk_"))
			continue;
		if ((tp = strstr(buf, "/media/AiDisk_")) != NULL)
		{
			pc = *(tp+14);
			memset(pcn, 0, sizeof(pcn));
			if ((*(tp+15)) && ((*(tp+15)) != ' '))
				pcn[0] = *(tp+15);
			if ((*(tp+16)) && ((*(tp+16)) != ' '))
				pcn[1] = *(tp+16);
			pd = atoi(pcn);
			if (pc >= 97)    // 'a' is 97, chk if starts from 'a'... and write flag 'h'(has)
			{
				mnt_n[pc-97][pd] = 'h';
			}
			if (strstr(buf, "fuseblk") != NULL)
			{
				fs_n[pc-97][pd] = 'n';
			}
			else if (strstr(buf, "ufsd") != NULL)
			{
				fs_n[pc-97][pd] = 'n';
			}
			else if (strstr(buf, "vfat") != NULL)
			{
				fs_n[pc-97][pd] = 'f';
			}
			else if (strstr(buf, "ext2") != NULL)
				fs_n[pc-97][pd] = '2';
			else if (strstr(buf, "ext3") != NULL)
				fs_n[pc-97][pd] = '3';
		}
		memset(buf, 0, sizeof(buf));
	}

	fclose(fp_p);
	fclose(fp_m);

#if 0
	/* dump mnt_n/par_n */
	dbg("\n\n############partitions##############\n\n");
	for (i=0, j=0, k=0; i<2; ++k)
	//for (i=25, j=0, k=0; i<26; ++k)
	{
		dbg("(%d,%d)=[%c] ", i, j, par_n[i][j]);
		++j;
		if (j==20)
		{
			j = 0;
			++i;
			continue;
		}
		if (k == 5)
		{
			dbg("\n");
			k = 0;
		}
	}
	dbg("\n\n############mounts##############\n\n");
	for (i=0, j=0, k=0; i<2; ++k)
	//for (i=25, j=0, k=0; i<26; ++k)
	{
		dbg("(%d,%d)=[%c] ", i, j, mnt_n[i][j]);
		++j;
		if (j==20)
		{
			++i;
			j = 0;
			continue;
		}
		if (k == 5)
		{
			dbg("\n");
			k = 0;
		}
	}
	dbg("\n\n############fs##############\n\n");
	for (i=0, j=0, k=0; i<2; ++k)
	{
		dbg("(%d,%d)=[%c] ", i, j, fs_n[i][j]);
		++j;
		if (j==20)
		{
			++i;
			j = 0;
			continue;
		}
		if (k == 5)
		{
			dbg("\n");
			k = 0;
		}
	}
#endif

	switch(state) {
	case USB_PLUG_ON:
/*
		for (i=0; i<26; ++i)	// check disk w/ no partition table and mount it
		{
			if ((par_n[i][0] == 'h') && (par_n[i][1] != 'h') && (mnt_n[i][1] != 'h'))
			{
					new_mount = i;	// we mount here, so it is 'new mount'
					mnt_op(i, 1, OP_MOUNT);
			}
			for (j=2; j<20; ++j)	// no need to chk sdx0/1
			{
				if ((par_n[i][j] == 'h') && (mnt_n[i][j] != 'h'))
					mnt_op(i, j, OP_MOUNT);
			}
		}
*/
//		set_nv = 0;
		free_max = 0;
		for (i=0; i<26; ++i)	// set nvram: usb_mnt_first_path
		{
//			if (set_nv)
//				break;

			// test
#if 0
			if (((i == new_mount) && ((par_n[i][0] == 'h') && (par_n[i][1] != 'h') && (mnt_n[i][1] != 'h')) /*&& chk_safe_fs(need_choose_fs, fs_n[i][1])*/))	// tmp test
			{
				dbg("### SETNV case 2, skip it\n");	// tmp test
			}
#endif
#if 0
			if (((par_n[i][1] == 'h') || (par_n[i][0] == 'h')) && ((mnt_n[i][1] == 'h') || (mnt_n[i][0] == 'h')) /*&& chk_safe_fs(need_choose_fs, fs_n[i][1])*/)  // consider sdx1 first, storage maybe small flash
			{
				choose_case = 1;
				dbg("### SETNVRAM case 1\n");	// tmp test
				if (compete_choose(i, 1) > 0)
				{
					op_i = i;
					op_j = 1;
				}
			} 
			else
			{
				for (j=2; j<20; ++j)
				{
					if ((par_n[i][j] == 'h') && (mnt_n[i][j] == 'h') /*&& chk_safe_fs(need_choose_fs, fs_n[i][j])*/)
					{
						choose_case = 3;
						dbg("### SETNVRAM case 3\n");	// tmp test
						if (compete_choose(i, j) > 0)
						{
							op_i = i;
							op_j = j;
						}
					}
				}
			}
#endif
			for (j=0; j<20; ++j)
			{
				if ((j == 0) && (par_n[i][j] == 'h') && (par_n[i][1] != 'h') && (mnt_n[i][1] == 'h'))
				{
					choose_case = 4;
					dbg("### SETNVRAM case 4\n");	// tmp test
					if (compete_choose(i, 1) > 0)
					{
						op_i = i;
						op_j = 1;
					}
				}
				else if ((par_n[i][j] == 'h') && (mnt_n[i][j] == 'h') /*&& chk_safe_fs(need_choose_fs, fs_n[i][j])*/)
				{
					choose_case = 3;
					dbg("### SETNVRAM case 3\n");	// tmp test
					if (compete_choose(i, j) > 0)
					{
						op_i = i;
						op_j = j;
					}
				}
			}
		}
		dbg("NV case is %d(%d, %d)\n", choose_case, op_i, op_j);	// tmp test

		if (	((op_i >= 0) && (op_j >= 0)) && 
			(!is_apps_running() || !swap_check()) && 
			!strlen(nvram_safe_get("usb_mnt_first_path")))
		{
			set_mnt_type(fs_n[op_i][op_j]);
			mnt_op(op_i, op_j, OP_SETNVRAM);
		}
		else
			dbg("OP SETNVRAM fail\n");	

		break;
/*
	case USB_PLUG_OFF:
		for (i=0; i<26; ++i)
		{
			for (j=1; j<20; ++j)
			{
				if (mnt_n[i][j] == 'h')
					mnt_op(i, j, OP_UMOUNT);
			}
		}
		break;
*/
	default:
		break;
	}
}

void
set_dev_class(char *dev, int *num)
{
	//dbg("set dev class:[%c][%c]\n", dev[0], dev[1]);	// tmp test
	if ((dev[0] == '0') && (dev[1] =='0'))
		*num = USB_CLS_PER_INTERFACE;
	else if ((dev[0] == '0') && (dev[1] =='1'))
	{
		*num = USB_CLS_AUDIO;
//		nvram_set("usb_path1", "audio");
	}
	else if ((dev[0] == '0') && (dev[1] =='2'))
	{
		*num = USB_CLS_COMM;
//		nvram_set("usb_path1", "comm");
	}
	else if ((dev[0] == '0') && (dev[1] =='3'))
	{
		*num = USB_CLS_HID;
//		nvram_set("usb_path1", "hid");
	}
	else if ((dev[0] == '0') && (dev[1] =='5'))
	{
		*num = USB_CLS_PHYSICAL;
//		nvram_set("usb_path1", "physical");
	}
	else if ((dev[0] == '0') && (dev[1] =='6'))
	{
		*num = USB_CLS_STILL_IMAGE;
//		nvram_set("usb_path1", "image");
	}
	else if ((dev[0] == '0') && (dev[1] =='7'))
	{
		*num = USB_CLS_PRINTER;
//		nvram_set("usb_path1", "printer");
	}
	else if ((dev[0] == '0') && (dev[1] =='8'))
	{
		*num = USB_CLS_MASS_STORAGE;
//		nvram_set("usb_path1", "storage");
	}
	else if ((dev[0] == '0') && (dev[1] =='9'))
	{
		*num = USB_CLS_HUB;
//		nvram_set("usb_path1", "hub");
	}
	else if ((dev[0] == '0') && (dev[1] =='a'))
	{
		*num = USB_CLS_CDC_DATA;
//		nvram_set("usb_path1", "cdc_data");
	}
	else if ((dev[0] == '0') && (dev[1] =='b'))
	{
		*num = USB_CLS_CSCID;
//		nvram_set("usb_path1", "cscid");
	}
	else if ((dev[0] == '0') && (dev[1] =='d'))
		*num = USB_CLS_CONTENT_SEC;
	else if ((dev[0] == '0') && (dev[1] =='e'))
	{
		*num = USB_CLS_VIDEO;
//		nvram_set("usb_path1", "video");
	}
	else if ((dev[0] == 'e') && (dev[1] =='0'))
	{
		*num = USB_CLS_WIRELESS_CONTROLLER;
//		nvram_set("usb_path1", "w_controller");
	}
	else if ((dev[0] == 'e') && (dev[1] =='f'))
	{
		*num = USB_CLS_MISC;
//		nvram_set("usb_path1", "misc");
	}
	else if ((dev[0] == 'f') && (dev[1] =='e'))
	{
		*num = USB_CLS_APP_SPEC;
//		nvram_set("usb_path1", "app_spec");
	}
	else if ((dev[0] == 'f') && (dev[1] =='f'))
	{
		*num = USB_CLS_VENDOR_SPEC;
//		nvram_set("usb_path1", "vendor_spec");
	}
	else
	{
		*num = USB_CLS_VENDOR_SPEC;
//		nvram_set("usb_path1", "vendor_spec");
	}
	//dbg("get usb_path is %s\n", nvram_safe_get("usb_path1"));	// tmp test
}

#ifndef RTCONFIG_USB_MODEM
int
get_dev_info(int *dev_class, char *product_id, char *vendor_id, char *prod_id)
{
	dbg("%s()...\n", __FUNCTION__);

	int fd, i, dev_num = 0;
	char *skey_1 = "I:*", *skey_2 = "Cls=", *skey_3 = "P:", *skey_4 = "Vendor=", *skey_5 = "ProdID=", *skey_6 = "Rev=";
	char *skey_7="Cls=07(print)", *skey_8 = "S:  Manufacturer=", *skey_9 = "S:  Product=", *skey_10 = "S:  SerialNumber=";
	char *skey_11 = "Port=", *skey_12 = "T:", *bp, *tmp_p;
	char buf[8192], b_class[3], vendor[5], prodid[5], rev[6], tmp_name[100], port[3];

	dbg("get dev info\n");	// tmp test
	if ((fd=open(usb_dev_file, O_RDONLY)) <= 0)
	{
		dbg("open usb devices fail\n");
		dbg("...%s()\n", __FUNCTION__);
		return 0;
	}

	memset(buf, 0, sizeof(buf));
	if (read(fd, buf, sizeof(buf)) <= 0)
	{
		dbg("read usb devices fail\n");
		close(fd);
		dbg("...%s()\n", __FUNCTION__);
		return 0;
	}
	close(fd);

/*
T:  Bus=01 Lev=01 Prnt=01 Port=00 Cnt=01 Dev#=  5 Spd=12  MxCh= 0
D:  Ver= 2.00 Cls=00(>ifc ) Sub=00 Prot=00 MxPS=64 #Cfgs=  1
P:  Vendor=0411 ProdID=0157 Rev= 1.03
S:  Manufacturer=BUFFALO 
S:  Product=HD-PEU2
S:  SerialNumber=57442D575845333039455934
C:* #Ifs= 1 Cfg#= 1 Atr=c0 MxPwr=  2mA
I:* If#= 0 Alt= 0 #EPs= 2 Cls=08(stor.) Sub=06 Prot=50 Driver=usb-storage
E:  Ad=81(I) Atr=02(Bulk) MxPS= 512 Ivl=0ms
E:  Ad=02(O) Atr=02(Bulk) MxPS= 512 Ivl=0ms
*/

	bp = buf;
//	while ((tmp_p = strstr(bp, skey_3)) != NULL)
	while ((tmp_p = strstr(bp, skey_12)) != NULL)
	{
		bp = tmp_p + 1;

		if (++dev_num <= 2)
			continue;

		/* get port */
		memset(port, 0, sizeof(port));
		tmp_p = strstr(bp, skey_11);
		if (tmp_p)
		{
			port[0] = *(tmp_p + 5);
			port[1] = *(tmp_p + 6);
		}

		if ((tmp_p = strstr(bp, skey_3)) == NULL)
			continue;
		else
			bp = tmp_p + 1;

		/* get vendor */
		memset(vendor, 0, sizeof(vendor));
		memset(prodid, 0, sizeof(prodid));
		memset(rev, 0, sizeof(rev));
		tmp_p = strstr(bp, skey_4);
		if (tmp_p)
		{
			vendor[0] = *(tmp_p + 7);
			vendor[1] = *(tmp_p + 8);
			vendor[2] = *(tmp_p + 9);
			vendor[3] = *(tmp_p + 10);
		}

		/* get productid */
		tmp_p = strstr(bp, skey_5);
		if (tmp_p)
		{
			prodid[0] = *(tmp_p + 7);
			prodid[1] = *(tmp_p + 8);
			prodid[2] = *(tmp_p + 9);
			prodid[3] = *(tmp_p + 10);
		}

		/* get rev */
		tmp_p = strstr(bp, skey_6);
		if (tmp_p)
		{
			rev[0] = *(tmp_p + 4);
			rev[1] = *(tmp_p + 5);
			rev[2] = *(tmp_p + 6);
			rev[3] = *(tmp_p + 7);
			rev[4] = *(tmp_p + 8);
		}

		sprintf(product_id, "%s/%s/%s", vendor, prodid, rev);
		sprintf(vendor_id, "%s", vendor);
		sprintf(prod_id, "%s", prodid);
//		dbg("set product_id as %s/%s/%s\n", vendor, prodid, rev);	// tmp test

		/* get manufact */
		tmp_p = strstr(bp, skey_8);
		memset(tmp_name, 0, sizeof(tmp_name));
		if (tmp_p)
		{
			tmp_p+=strlen(skey_8);
			for (i=0; (*(tmp_p+i)!='\n')&&(i<100); ++i)
				tmp_name[i] = *(tmp_p+i);
			dbg("get Manufacturer=%s\n", tmp_name);	// tmp test
			nvram_set("usb_Manufacturer", tmp_name);
		}
		else
			nvram_set("usb_Manufacturer", "unknown");

		/* get product name */
		tmp_p = strstr(bp, skey_9);
		memset(tmp_name, 0, sizeof(tmp_name));
		if (tmp_p)
		{
			tmp_p+=strlen(skey_9);
			for (i=0; (*(tmp_p+i)!='\n')&&(i<100); ++i)
				tmp_name[i] = *(tmp_p+i);
			dbg("get productname=%s\n", tmp_name);      // tmp test
			nvram_set("usb_Product", tmp_name);
		}
		else
			nvram_set("usb_Product", "usb_3g_dongle");

		/* get serialnum */
		tmp_p = strstr(bp, skey_10);
		memset(tmp_name, 0, sizeof(tmp_name));
		if (tmp_p)
		{
			tmp_p+=strlen(skey_10);
			for (i=0; (*(tmp_p+i)!='\n')&&(i<100); ++i)
				tmp_name[i] = *(tmp_p+i);
			dbg("get serialnum=%s\n", tmp_name);      // tmp test
			nvram_set("usb_SerialNumber", tmp_name);
		}
		else
			nvram_set("usb_SerialNumber", "unknown");

		/* get class */
		tmp_p = strstr(bp, skey_2);
		memset(b_class, 0, sizeof(b_class));
		if (tmp_p)
		{
			b_class[0] = *(tmp_p + 4);
			b_class[1] = *(tmp_p + 5);
		}
		else
		{
			b_class[0] = 'f';
			b_class[1] = 'f';
		}

		if (strstr(bp, skey_7))
		{
			b_class[0] = '0';
			b_class[1] = '7';
		}

		/* set device class */
		set_dev_class(b_class, dev_class);

		if ((strcmp(nvram_safe_get("wan_proto"), "3g")==0) && (strlen(nvram_safe_get("Dev3G")) > 0))
		{
			*dev_class = USB_CLS_3GDEV;
		}
	}

//	close(fd);

	if (dev_num <= 2)
	{
		dbg("...%s()\n", __FUNCTION__);
		return 0;
	}

	dbg("get productid=%s, devnum=%d, port=%s\n", product_id, dev_num, port);	// tmp test

	dbg("...%s()\n", __FUNCTION__);

	return 1;
}
#endif

int
get_dev_info_storage(const char *buf)
{
	dbg("%s()...\n", __FUNCTION__);

	int i= 0;
	char *skey_0 = "Lev=", *skey_1 = "Port=", *skey_2 = "I:*", *skey_3 = "Cls=", *skey_4 = "Vendor=";
	char *skey_5 = "ProdID=", *skey_6 = "S:  Product=", *bp, *tmp_p;
	char *skey_7 = "Cls=08(stor.) Sub=06 Prot=50 Driver=usb-storage";
	char b_class[3], vendid[5], prodid[5], port[3], level[3], productname[100];

/*
T:  Bus=01 Lev=01 Prnt=01 Port=00 Cnt=01 Dev#=  5 Spd=12  MxCh= 0
D:  Ver= 2.00 Cls=00(>ifc ) Sub=00 Prot=00 MxPS=64 #Cfgs=  1
P:  Vendor=0411 ProdID=0157 Rev= 1.03
S:  Manufacturer=BUFFALO 
S:  Product=HD-PEU2
S:  SerialNumber=57442D575845333039455934
C:* #Ifs= 1 Cfg#= 1 Atr=c0 MxPwr=  2mA
I:* If#= 0 Alt= 0 #EPs= 2 Cls=08(stor.) Sub=06 Prot=50 Driver=usb-storage
E:  Ad=81(I) Atr=02(Bulk) MxPS= 512 Ivl=0ms
E:  Ad=02(O) Atr=02(Bulk) MxPS= 512 Ivl=0ms
*/

	bp = buf;

	/* get level */
	memset(level, 0x0, sizeof(level));
	tmp_p = strstr(bp, skey_0);
	if (tmp_p)
		memcpy(level, tmp_p + 4, 2);

	/* get port */
	memset(port, 0x0, sizeof(port));
	tmp_p = strstr(bp, skey_1);
	if (tmp_p)
		memcpy(port, tmp_p + 5, 2);
/*			
	{
		port[0] = *(tmp_p + 5);
		port[1] = *(tmp_p + 6);
	}
*/

	/* get vendor id */
	memset(vendid, 0x0, sizeof(vendid));
	tmp_p = strstr(bp, skey_4);
	if (tmp_p)
	{
		vendid[0] = *(tmp_p + 7);
		vendid[1] = *(tmp_p + 8);
		vendid[2] = *(tmp_p + 9);
		vendid[3] = *(tmp_p + 10);
	}

	/* get product id */
	memset(prodid, 0x0, sizeof(prodid));
	tmp_p = strstr(bp, skey_5);
	if (tmp_p)
	{
		prodid[0] = *(tmp_p + 7);
		prodid[1] = *(tmp_p + 8);
		prodid[2] = *(tmp_p + 9);
		prodid[3] = *(tmp_p + 10);
	}

	/* get product name */
	memset(productname, 0x0, sizeof(productname));
	tmp_p = strstr(bp, skey_6);
	if (tmp_p)
	{
		tmp_p+=strlen(skey_6);
		for (i=0; (*(tmp_p+i)!='\n')&&(i<sizeof(productname)); ++i)
		productname[i] = *(tmp_p+i);
	}

	/* get class */
	memset(b_class, 0x0, sizeof(b_class));
	tmp_p = strstr(bp, skey_7);
	if (tmp_p)
		memcpy(b_class, "08", 2);
	else
	{
		memcpy(b_class, "00", 2);

		dbg("...%s()\n", __FUNCTION__);

		return 0;
	}

	/* set device class */
//	set_dev_class(b_class, dev_class);

	dbg("get product %s,  vid/pid: %s/%s, level/port: %s/%s, class: %s\n", productname, vendid, prodid, level, port, b_class);

	dbg("...%s()\n", __FUNCTION__);

	return 1;
}

int
parse_proc_bus_usb_devices()
{
	int fd, count, last;
	int count2;
	char buf[8192], buf2[2048];
	char *bufp, *bufp_old, *key_T = "T:  ";

	count = count2 = last = 0;

	if ((fd=open("/proc/bus/usb/devices", O_RDONLY)) <= 0)
	{
		dbg("open usb devices fail\n");
		return 0;
	}

	memset(buf, 0, sizeof(buf));
	if ((count = read(fd, buf, sizeof(buf))) <= 0)
	{
		dbg("read usb devices fail\n");
		close(fd);
		return 0;
	}
	close(fd);

	bufp_old = buf;

	while ((bufp = strstr(bufp_old, key_T)) != NULL || (last > 0))
	{
		if (count2 == 0)
		{
			bufp_old = bufp + 1;
			count2++;
			continue;
		}

		memset(buf2, 0x0, 2048);

		if (last > 0)
		{
			if (count2 > 2)
			{
				strncpy(buf2, bufp_old - 1, count - ((bufp_old - 1) - buf) - 1);
				dbg("%s\n\n", buf2);
//				get_dev_info_storage(buf2);
			}
			break;
		}
		else if (!strstr(bufp + 1, key_T))
		{
			last = 1;
		}
		else
			last = 0;

		if (count2 > 2)
		{
			strncpy(buf2, bufp_old - 1, bufp - bufp_old - 1);
			dbg("%s\n\n", buf2);
//			get_dev_info_storage(buf2);
		}
		bufp_old = bufp + 1;
		count2++;
	}

	return 1;
}

#ifndef RTCONFIG_USB_MODEM
int write_genconn()
{
	FILE *fp;

	unlink("/etc_ro/ppp/3g/Generic_conn.scr");
	fp = fopen("/etc_ro/ppp/3g/Generic_conn.scr", "w+");

	fprintf(fp, "opengt\n");
	fprintf(fp, "set com 115200n81\n");
	fprintf(fp, "set senddelay 0.05\n");
	fprintf(fp, "waitquiet 1 0.2\n");
	fprintf(fp, "send \"ATZ^m\"\n");
	fprintf(fp, "waitfor 10 \"OK\",\"ERR\",\"ERROR\"\n");
	fclose(fp);

	system("echo 'if % = -1 goto timeerror' >> /etc_ro/ppp/3g/Generic_conn.scr");
	system("echo 'if % = 0 goto next1' >> /etc_ro/ppp/3g/Generic_conn.scr");
	system("echo 'if % = 1 goto error' >> /etc_ro/ppp/3g/Generic_conn.scr");
	system("echo 'if % = 2 goto error' >> /etc_ro/ppp/3g/Generic_conn.scr");

	fp = fopen("/etc_ro/ppp/3g/Generic_conn.scr", "a");
	fprintf(fp, ":next1\n");
	fprintf(fp, "send \"AT+CFUN=1;+CGDCONT=1,\\\"IP\\\",\\\"%s\\\"^m\"\n", nvram_safe_get("hsdpa_apn"));
	fprintf(fp, "waitfor 10 \"OK\",\"ERR\",\"ERROR\"\n");
	fclose(fp);

	system("echo 'if % = -1 goto timeerror' >> /etc_ro/ppp/3g/Generic_conn.scr");
	system("echo 'if % = 0 goto next2' >> /etc_ro/ppp/3g/Generic_conn.scr");
	system("echo 'if % = 1 goto error' >> /etc_ro/ppp/3g/Generic_conn.scr");
	system("echo 'if % = 2 goto error' >> /etc_ro/ppp/3g/Generic_conn.scr");

	fp = fopen("/etc_ro/ppp/3g/Generic_conn.scr", "a");
	fprintf(fp, ":next2\n");
	fprintf(fp, "send \"ATD%s^m\"\n", nvram_safe_get("hsdpa_dialnum"));
	fprintf(fp, "waitfor 10 \"CONNECT\",\"ERR\",\"ERROR\"\n");
	fclose(fp);

	system("echo 'if % = -1 goto timeerror' >> /etc_ro/ppp/3g/Generic_conn.scr");
	system("echo 'if % = 0 goto next3' >> /etc_ro/ppp/3g/Generic_conn.scr");
	system("echo 'if % = 1 goto error' >> /etc_ro/ppp/3g/Generic_conn.scr");
	system("echo 'if % = 2 goto error' >> /etc_ro/ppp/3g/Generic_conn.scr");

	fp = fopen("/etc_ro/ppp/3g/Generic_conn.scr", "a");
	fprintf(fp, ":next3\n");
	fprintf(fp, "print \"CONNECTED\\n\"\n");
	fprintf(fp, "exit 0\n");
	fprintf(fp, ":error\n");
	fprintf(fp, "print \"CONNECT ERROR\\n\"\n");
	fprintf(fp, "exit 1\n");
	fprintf(fp, ":timeerror\n");
	fprintf(fp, "print \"CONNECT TIMEOUT\\n\"\n");
	fprintf(fp, "exit 1\n");
	fclose(fp);

	return 0;
}

int write_3g_conf(FILE *fp, int dno, int aut, char *vid, char *pid)
{
	switch(dno)
	{
	case SN_MU_Q101:
		fprintf(fp, "DefaultVendor=  0x0408\n");
		fprintf(fp, "DefaultProduct= 0x1000\n");
		fprintf(fp, "TargetVendor=   0x0408\n");
		fprintf(fp, "TargetProduct=  0xea02\n");
		fprintf(fp, "MessageEndpoint=0x05\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_OPTION_ICON225:
		fprintf(fp, "DefaultVendor=  0x0af0\n");
		fprintf(fp, "DefaultProduct= 0x6971\n");
		fprintf(fp, "TargetClass=    0xff\n");
		fprintf(fp, "MessageContent=\"555342431223456780100000080000601000000000000000000000000000000\"\n");
		break;
	case SN_Option_GlobeSurfer_Icon:
		fprintf(fp, "DefaultVendor=  0x05c6\n");
		fprintf(fp, "DefaultProduct= 0x1000\n");
		fprintf(fp, "TargetVendor=   0x0af0\n");
		fprintf(fp, "TargetProduct=  0x6600\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000601000000000000000000000000000000\"\n");
		break;
	case SN_Option_GlobeSurfer_Icon72:
		fprintf(fp, "DefaultVendor=  0x05c6\n");
		fprintf(fp, "DefaultProduct= 0x1000\n");
		fprintf(fp, "TargetVendor=   0x0af0\n");
		fprintf(fp, "TargetProduct=  0x6901\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000601000000000000000000000000000000\"\n");
		break;
	case SN_Option_GlobeTrotter_GT_MAX36:
		fprintf(fp, "DefaultVendor=  0x05c6\n");
		fprintf(fp, "DefaultProduct= 0x1000\n");
		fprintf(fp, "TargetVendor=   0x0af0\n");
		fprintf(fp, "TargetProduct=  0x6600\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000601000000000000000000000000000000\"\n");
		fprintf(fp, "ResponseNeeded=1\n");
		break;
	case SN_Option_GlobeTrotter_GT_MAX72:
		fprintf(fp, "DefaultVendor=  0x05c6\n");
		fprintf(fp, "DefaultProduct= 0x1000\n");
		fprintf(fp, "TargetVendor=   0x0af0\n");
		fprintf(fp, "TargetProduct=  0x6701");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000601000000000000000000000000000000\"\n");
		break;
	case SN_Option_GlobeTrotter_EXPRESS72:
		fprintf(fp, "DefaultVendor=  0x05c6\n");
		fprintf(fp, "DefaultProduct= 0x1000\n");
		fprintf(fp, "TargetVendor=   0x0af0\n");
		fprintf(fp, "TargetProduct=  0x6701\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000601000000000000000000000000000000\"\n");
		fprintf(fp, "ResponseNeeded=1\n");
		break;
	case SN_Option_iCON210:
		fprintf(fp, "DefaultVendor=  0x1e0e\n");
		fprintf(fp, "DefaultProduct= 0xf000\n");
		fprintf(fp, "TargetVendor=   0x1e0e\n");
		fprintf(fp, "TargetProduct=  0x9000\n");
		fprintf(fp, "MessageContent=\"555342431234567800000000000006bd000000020000000000000000000000\"\n");
		fprintf(fp, "ResponseNeeded=1\n");
		break;
	case SN_Option_GlobeTrotter_HSUPA_Modem:
		fprintf(fp, "DefaultVendor=  0x0af0\n");
		fprintf(fp, "DefaultProduct= 0x7011\n");
		fprintf(fp, "TargetClass=    0xff\n");
		fprintf(fp, "MessageContent=\"55534243785634120100000080000601000000000000000000000000000000\"\n");
		break;
	case SN_Option_iCON401:
		fprintf(fp, "DefaultVendor=  0x0af0\n");
		fprintf(fp, "DefaultProduct= 0x7401\n");
		fprintf(fp, "TargetClass=    0xff\n");
		fprintf(fp, "MessageContent=\"55534243785634120100000080000601000000000000000000000000000000\"\n");
		fprintf(fp, "ResponseNeeded=1\n");
		break;
	case SN_Vodafone_K3760:
		fprintf(fp, "DefaultVendor=  0x0af0\n");
		fprintf(fp, "DefaultProduct= 0x7501\n");
		fprintf(fp, "TargetClass=    0xff\n");
		fprintf(fp, "MessageContent=\"55534243785634120100000080000601000000000000000000000000000000\"\n");
		break;
	case SN_ATT_USBConnect_Quicksilver:
		fprintf(fp, "DefaultVendor=  0x0af0\n");
		fprintf(fp, "DefaultProduct= 0xd033\n");
		fprintf(fp, "TargetClass=    0xff\n");
		fprintf(fp, "MessageContent=\"55534243785634120100000080000601000000000000000000000000000000\"\n");
		break;
	case SN_Huawei_E169:
		fprintf(fp, "DefaultVendor=  0x12d1;\n");
		fprintf(fp, "DefaultProduct= 0x1001\n");
		fprintf(fp, "DetachStorageOnly=1\n");
		fprintf(fp, "HuaweiMode=1\n");
		break;
	case SN_Huawei_E220:
		fprintf(fp, "DefaultVendor=  0x12d1;\n");
		fprintf(fp, "DefaultProduct= 0x1003\n");
		fprintf(fp, "TargetClass=    0xff\n");
		fprintf(fp, "DetachStorageOnly=1\n");
		fprintf(fp, "HuaweiMode=1\n");
		break;
	case SN_Huawei_E180:
		fprintf(fp, "DefaultVendor=  0x12d1\n");
		fprintf(fp, "DefaultProduct= 0x1414\n");
		fprintf(fp, "TargetClass=    0xff\n");
		fprintf(fp, "DetachStorageOnly=1\n");
		fprintf(fp, "HuaweiMode=1\n");
		break;
	case SN_Huawei_E630:
		fprintf(fp, "DefaultVendor=  0x1033\n");
		fprintf(fp, "DefaultProduct= 0x0035\n");
		fprintf(fp, "TargetVendor=   0x12d1\n");
		fprintf(fp, "TargetProduct=  0x1003\n");
		fprintf(fp, "HuaweiMode=1\n");
		fprintf(fp, "DetachStorageOnly=1\n");
		break;
	case SN_Huawei_E270:
		fprintf(fp, "DefaultVendor=  0x12d1\n");
		fprintf(fp, "DefaultProduct= 0x1446\n");
		fprintf(fp, "TargetVendor=   0x12d1\n");
		fprintf(fp, "TargetProduct=  0x14ac\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
		break;
	case SN_Huawei_E1550:
		fprintf(fp, "DefaultVendor=  0x12d1\n");
		fprintf(fp, "DefaultProduct= 0x1446\n");
		fprintf(fp, "TargetVendor=   0x12d1\n");
		fprintf(fp, "TargetProduct=  0x1001\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
		break;
	case SN_Huawei_E1612:
		fprintf(fp, "DefaultVendor=  0x12d1\n");
		fprintf(fp, "DefaultProduct= 0x1446\n");
		fprintf(fp, "TargetVendor=   0x12d1\n");
		fprintf(fp, "TargetProduct=  0x1406\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
		break;
	case SN_Huawei_E1690:
		fprintf(fp, "DefaultVendor=  0x12d1\n");
		fprintf(fp, "DefaultProduct= 0x1446\n");
		fprintf(fp, "TargetVendor=   0x12d1\n");
		fprintf(fp, "TargetProduct=  0x140c\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
		break;
	case SN_Huawei_K3765:
		fprintf(fp, "DefaultVendor=  0x12d1\n");
		fprintf(fp, "DefaultProduct= 0x1520\n");
		fprintf(fp, "TargetVendor=   0x12d1\n");
		fprintf(fp, "TargetProduct=  0x1465\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
		break;
	case SN_Huawei_K4505:
		fprintf(fp, "DefaultVendor=  0x12d1\n");
		fprintf(fp, "DefaultProduct= 0x1521\n");
		fprintf(fp, "TargetVendor=   0x12d1\n");
		fprintf(fp, "TargetProduct=  0x1464\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000011060000000000000000000000000000\"\n");
		break;
	case SN_ZTE_MF620:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0x2000\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0x0001\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000600000000000000000000000000000000\"\n");
		break;
	case SN_ZTE_MF622:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0x2000\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0x0002\n");
		fprintf(fp, "MessageContent=\"55534243f8f993882000000080000a85010101180101010101000000000000\"\n");
		break;
	case SN_ZTE_MF628:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0x2000\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0x0015\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000030000000000000000000000\"\n");
		fprintf(fp, "ResponseNeeded=1\n");
		break;
	case SN_ZTE_MF626:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0x2000\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0x0031\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000030000000000000000000000\"\n");
		break;
	case SN_ZTE_AC8710:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0xfff5\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0xffff\n");
		fprintf(fp, "MessageContent=\"5553424312345678c00000008000069f030000000000000000000000000000\"\n");
		break;
	case SN_ZTE_AC2710:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0xfff5\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0xffff\n");
		fprintf(fp, "MessageContent=\"5553424312345678c00000008000069f010000000000000000000000000000\"\n");
		break;
	case SN_ZTE6535_Z:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0x2000\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0x0052\n");
		fprintf(fp, "MessageContent=\"55534243123456782000000080000c85010101180101010101000000000000\"\n");
		break;
	case SN_ZTE_K3520_Z:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0x2000\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0x0055\n");
		fprintf(fp, "MessageContent=\"55534243123456782000000080000c85010101180101010101000000000000\"\n");
		break;
	case SN_ZTE_MF110:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0x0053\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0x0031\n");
		fprintf(fp, "MessageContent=\"55534243123456782000000080000c85010101180101010101000000000000\"\n");
		break;
	case SN_ZTE_K3565:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0x2000\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0x0063\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_ONDA_MT503HS:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0x2000\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0x0002\n");
		fprintf(fp, "MessageContent=\"55534243b0c8dc812000000080000a85010101180101010101000000000000\"\n");
		break;
	case SN_ONDA_MT505UP:
		fprintf(fp, "DefaultVendor=  0x19d2\n");
		fprintf(fp, "DefaultProduct= 0x2000\n");
		fprintf(fp, "TargetVendor=   0x19d2\n");
		fprintf(fp, "TargetProduct=  0x0002\n");
		fprintf(fp, "MessageContent=\"55534243123456780000010080000a28000000001c00002000000000000000\"\n");
		break;
	case SN_Novatel_Wireless_Ovation_MC950D:
		fprintf(fp, "DefaultVendor=  0x1410\n");
		fprintf(fp, "DefaultProduct= 0x5010\n");
		fprintf(fp, "TargetVendor=   0x1410\n");
		fprintf(fp, "TargetProduct=  0x4400\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_Novatel_U727:
		fprintf(fp, "DefaultVendor=  0x1410\n");
		fprintf(fp, "DefaultProduct= 0x5010\n");
		fprintf(fp, "TargetVendor=   0x1410\n");
		fprintf(fp, "TargetProduct=  0x4100\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_Novatel_MC990D:
		fprintf(fp, "DefaultVendor=  0x1410\n");
		fprintf(fp, "DefaultProduct= 0x5020\n");
		fprintf(fp, "Interface=      5\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_Novatel_U760:
		fprintf(fp, "DefaultVendor=  0x1410\n");
		fprintf(fp, "DefaultProduct= 0x5030\n");
		fprintf(fp, "TargetVendor=   0x1410\n");
		fprintf(fp, "TargetProduct=  0x6000\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_Alcatel_X020:
		fprintf(fp, "DefaultVendor=  0x1c9e\n");
		fprintf(fp, "DefaultProduct= 0x1001\n");
		fprintf(fp, "TargetVendor=   0x1c9e\n");
		fprintf(fp, "TargetProduct=  0x6061\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000606f50402527000000000000000000000\"\n");
		break;
	case SN_Alcatel_X200:
		fprintf(fp, "DefaultVendor=  0x1bbb\n");
		fprintf(fp, "DefaultProduct= 0xf000\n");
		fprintf(fp, "TargetVendor=   0x1bbb\n");
		fprintf(fp, "TargetProduct=  0x0000\n");
		fprintf(fp, "MessageContent=\"55534243123456788000000080000606f50402527000000000000000000000\"\n");
		break;
	case SN_AnyDATA_ADU_500A:
		fprintf(fp, "DefaultVendor=  0x05c6\n");
		fprintf(fp, "DefaultProduct= 0x1000\n");
		fprintf(fp, "TargetVendor=   0x16d5\n");
		fprintf(fp, "TargetProduct=  0x6502\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_BandLuxe_C120:
		fprintf(fp, "DefaultVendor=  0x1a8d\n");
		fprintf(fp, "DefaultProduct= 0x1000\n");
		fprintf(fp, "TargetVendor=   0x1a8d\n");
		fprintf(fp, "TargetProduct=  0x1002\n");
		fprintf(fp, "MessageContent=\"55534243123456781200000080000603000000020000000000000000000000\"\n");
		fprintf(fp, "ResponseNeeded=1\n");
		break;
	case SN_Solomon_S3Gm660:
		fprintf(fp, "DefaultVendor=  0x1dd6\n");
		fprintf(fp, "DefaultProduct= 0x1000\n");
		fprintf(fp, "TargetVendor=   0x1dd6\n");
		fprintf(fp, "TargetProduct=  0x1002\n");
		fprintf(fp, "MessageContent=\"55534243123456781200000080000603000000020000000000000000000000\"\n");
		fprintf(fp, "ResponseNeeded=1\n");
		break;
	case SN_C_motechD50:
		fprintf(fp, "DefaultVendor=  0x16d8\n");
		fprintf(fp, "DefaultProduct= 0x6803\n");
		fprintf(fp, "TargetVendor=   0x16d8\n");
		fprintf(fp, "TargetProduct=  0x680a\n");
		fprintf(fp, "MessageContent=\"555342431234567824000000800008ff524445564348470000000000000000\"\n");
		break;
	case SN_C_motech_CGU628:
		fprintf(fp, "DefaultVendor=  0x16d8\n");
		fprintf(fp, "DefaultProduct= 0xf000\n");
		fprintf(fp, "TargetVendor=   0x16d8\n");
		fprintf(fp, "TargetProduct=  0x6006\n");
		fprintf(fp, "MessageContent=\"55534243d85dd88524000000800008ff524445564348470000000000000000\"\n");
		break;
	case SN_Toshiba_G450:
		fprintf(fp, "DefaultVendor=  0x0930\n");
		fprintf(fp, "DefaultProduct= 0x0d46\n");
		fprintf(fp, "TargetVendor=   0x0930\n");
		fprintf(fp, "TargetProduct=  0x0d45\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_UTStarcom_UM175:
		fprintf(fp, "DefaultVendor=  0x106c\n");
		fprintf(fp, "DefaultProduct= 0x3b03\n");
		fprintf(fp, "TargetVendor=   0x106c\n");
		fprintf(fp, "TargetProduct=  0x3715\n");
		fprintf(fp, "MessageContent=\"555342431234567824000000800008ff024445564348470000000000000000\"\n");
		break;
	case SN_Hummer_DTM5731:
		fprintf(fp, "DefaultVendor=  0x1ab7\n");
		fprintf(fp, "DefaultProduct= 0x5700\n");
		fprintf(fp, "TargetVendor=   0x1ab7\n");
		fprintf(fp, "TargetProduct=  0x5731\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_A_Link_3GU:
		fprintf(fp, "DefaultVendor=  0x1e0e\n");
		fprintf(fp, "DefaultProduct= 0xf000\n");
		fprintf(fp, "TargetVendor=   0x1e0e\n");
		fprintf(fp, "TargetProduct=  0x9200\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_Sierra_Wireless_Compass597:
		fprintf(fp, "DefaultVendor=  0x1199\n");
		fprintf(fp, "DefaultProduct= 0x0fff\n");
		fprintf(fp, "TargetVendor=   0x1199\n");
		fprintf(fp, "TargetProduct=  0x0023\n");
		fprintf(fp, "SierraMode=1\n");
		break;
	case SN_Sierra881U:
		fprintf(fp, "DefaultVendor=  0x1199\n");
		fprintf(fp, "DefaultProduct= 0x0fff\n");
		fprintf(fp, "TargetVendor=   0x1199\n");
		fprintf(fp, "TargetProduct=  0x6856\n");
		fprintf(fp, "SierraMode=1\n");
		break;
	case SN_Sony_Ericsson_MD400:
		fprintf(fp, "DefaultVendor=  0x0fce\n");
		fprintf(fp, "DefaultProduct= 0xd0e1\n");
		fprintf(fp, "TargetClass=    0x02\n");
		fprintf(fp, "SonyMode=1\n");
		fprintf(fp, "Configuration=2\n");
		break;
	case SN_LG_LDU_1900D:
		fprintf(fp, "DefaultVendor=  0x1004\n");
		fprintf(fp, "DefaultProduct= 0x1000\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000aff554d53434847000000000000000000\"\n");
		break;
	case SN_Samsung_SGH_Z810:
		fprintf(fp, "DefaultVendor=  0x05c6\n");
		fprintf(fp, "DefaultProduct= 0x1000\n");
		fprintf(fp, "TargetVendor=   0x04e8\n");
		fprintf(fp, "TargetProduct=  0x6601\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000616000000000000000000000000000000\"\n");
		break;
	case SN_MobiData_MBD_200HU:
		fprintf(fp, "DefaultVendor=  0x1c9e\n");
		fprintf(fp, "DefaultProduct= 0xf000\n");
		fprintf(fp, "TargetVendor=   0x1c9e\n");
		fprintf(fp, "TargetProduct=  0x9000\n");
		fprintf(fp, "MessageContent=\"55534243123456788000000080000606f50402527000000000000000000000\"\n");
		break;
	case SN_ST_Mobile:
		fprintf(fp, "DefaultVendor=  0x1c9e\n");
		fprintf(fp, "DefaultProduct= 0xf000\n");
		fprintf(fp, "TargetVendor=   0x1c9e\n");
		fprintf(fp, "TargetProduct=  0x9063\n");
		fprintf(fp, "MessageContent=\"55534243123456788000000080000606f50402527000000000000000000000\"\n");
		break;
	case SN_MyWave_SW006:
		fprintf(fp, "DefaultVendor=  0x1c9e\n");
		fprintf(fp, "DefaultProduct= 0x9200\n");
		fprintf(fp, "TargetVendor=   0x1c9e\n");
		fprintf(fp, "TargetProduct=  0x9202\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000606f50402527000000000000000000000\"\n");
		break;
	case SN_Cricket_A600:
		fprintf(fp, "DefaultVendor=  0x1f28\n");
		fprintf(fp, "DefaultProduct= 0x0021\n");
		fprintf(fp, "TargetVendor=   0x1f28\n");
		fprintf(fp, "TargetProduct=  0x0020\n");
		fprintf(fp, "MessageContent=\"555342431234567824000000800108df200000000000000000000000000000\"\n");
		break;
	case SN_EpiValley_SEC7089:
		fprintf(fp, "DefaultVendor=  0x1b7d\n");
		fprintf(fp, "DefaultProduct= 0x0700\n");
		fprintf(fp, "TargetVendor=   0x1b7d\n");
		fprintf(fp, "TargetProduct=  0x0001\n");
		fprintf(fp, "MessageContent=\"555342431234567824000000800008FF05B112AEE102000000000000000000\"\n");
		break;
	case SN_Samsung_U209:
		fprintf(fp, "DefaultVendor=  0x04e8\n");
		fprintf(fp, "DefaultProduct= 0xf000\n");
		fprintf(fp, "TargetVendor=   0x04e8\n");
		fprintf(fp, "TargetProduct=  0x6601\n");
		fprintf(fp, "MessageContent=\"55534243123456780000000000000616000000000000000000000000000000\"\n");
		break;
	case SN_D_Link_DWM162_U5:
		fprintf(fp, "DefaultVendor=  0x05c6\n");
		fprintf(fp, "DefaultProduct= 0x2001\n");
		fprintf(fp, "TargetVendor=   0x1e0e\n");
		fprintf(fp, "TargetProduct=  0xce16\n");
		fprintf(fp, "MessageContent=\"55534243e0c26a85000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_Novatel_MC760:
		fprintf(fp, "DefaultVendor=  0x1410\n");
		fprintf(fp, "DefaultProduct= 0x5031\n");
		fprintf(fp, "TargetVendor=   0x1410\n");
		fprintf(fp, "TargetProduct=  0x6002\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_Philips_TalkTalk:
		fprintf(fp, "DefaultVendor=  0x0471\n");
		fprintf(fp, "DefaultProduct= 0x1237\n");
		fprintf(fp, "TargetVendor=   0x0471\n");
		fprintf(fp, "TargetProduct=  0x1234\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000030000000000000000000000\"\n");
		break;
	case SN_HuaXing_E600:
		fprintf(fp, "DefaultVendor=  0x0471\n");
		fprintf(fp, "DefaultProduct= 0x1237\n");
		fprintf(fp, "TargetVendor=   0x0471\n");
		fprintf(fp, "TargetProduct=  0x1206\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		fprintf(fp, "Configuration=2\n");
		break;
	case SN_C_motech_CHU_629S:
		fprintf(fp, "DefaultVendor=  0x16d8\n");
		fprintf(fp, "DefaultProduct= 0x700a\n");
		fprintf(fp, "TargetClass=0xff\n");
		fprintf(fp, "MessageContent=\"55534243123456782400000080000dfe524445564348473d4e444953000000\"\n");
		break;
	case SN_Sagem9520:
		fprintf(fp, "DefaultVendor=  0x1076\n");
		fprintf(fp, "DefaultProduct= 0x7f40\n");
		fprintf(fp, "TargetVendor=   0x1076\n");
		fprintf(fp, "TargetProduct=  0x7f00\n");
		fprintf(fp, "GCTMode=1\n");
		break;
	case SN_Nokia_CS15:
		fprintf(fp, "DefaultVendor=  0x0421\n");
		fprintf(fp, "DefaultProduct= 0x0610\n");
		fprintf(fp, "TargetVendor=   0x0421\n");
		fprintf(fp, "TargetProduct=  0x0612\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_Vodafone_MD950:
		fprintf(fp, "DefaultVendor=  0x0471\n");
		fprintf(fp, "DefaultProduct= 0x1210\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	case SN_Siptune_LM75:
		fprintf(fp, "DefaultVendor=  0x05c6\n");
		fprintf(fp, "DefaultProduct= 0xf000\n");
		fprintf(fp, "TargetVendor=   0x05c6\n");
		fprintf(fp, "TargetProduct=  0x9000\n");
		fprintf(fp, "MessageContent=\"5553424312345678000000000000061b000000020000000000000000000000\"\n");
		break;
	default:
		fprintf(fp, "\n");
		if (vid && pid)
		{
			fprintf(fp, "DefaultVendor=  0x%s\n", vid);
			fprintf(fp, "DefaultProduct= 0x%s\n", pid);
			if (strcmp(vid, "12d1") == 0)    // huawei
			{
				fprintf(fp, "DetachStorageOnly=1\n");
				fprintf(fp, "HuaweiMode=1\n");
			}
		}
		break;
	}

	return 0;
}

int init_3g_param(char *vid, char *pid)
{
	FILE *fp;
	char dev_vid[10], dev_pid[10];

	unlink("/etc_ro/usb/g3.conf");
	fp = fopen("/etc_ro/usb/g3.conf", "w+");
	if (!fp)
		return -1;

	memset(dev_vid, 0, sizeof(dev_vid));
	memset(dev_pid, 0, sizeof(dev_pid));
	sprintf(dev_vid, "0x%s", vid);
	sprintf(dev_pid, "0x%s", pid);
	nvram_set("dev_vid", dev_vid);
	nvram_set("dev_pid", dev_pid);

	if (nvram_match("Dev3G", "AUTO"))
	{
		nvram_set("d3g", "usb_3g_dongle");

		if ((strcmp(vid, "0408")==0) && ((strcmp(pid, "ea02")==0)||(strcmp(pid, "1000")==0)))
			write_3g_conf(fp, SN_MU_Q101, 1, vid, pid);
		else if ((strcmp(vid, "0af0")==0) && (strcmp(pid, "6971")==0))
		{
			nvram_set("d3g", "OPTION-ICON225");
			write_3g_conf(fp, SN_OPTION_ICON225, 1, vid, pid);
		}
		else if ((strcmp(vid, "05c6")==0) && (strcmp(pid, "1000")==0)) // also Option-GlobeSurfer-Icon72(may have new fw setting, bug not included here), Option-GlobeTrotter-GT-MAX36.....Option-Globexx series, AnyDATA-ADU-500A, Samsung-SGH-Z810, Vertex Wireless 100 Series
			write_3g_conf(fp, SN_Option_GlobeSurfer_Icon, 1, vid, pid);
		else if ((strcmp(vid, "1e0e")==0) && (strcmp(pid, "f000")==0))	// A-Link-3GU
			write_3g_conf(fp, SN_Option_iCON210, 1, vid, pid);
		else if ((strcmp(vid, "0af0")==0) && (strcmp(pid, "7011")==0))
		{
			nvram_set("d3g", "Option-GlobeTrotter-HSUPA-Modem");
			write_3g_conf(fp, SN_Option_GlobeTrotter_HSUPA_Modem, 1, vid, pid);
		}
		else if ((strcmp(vid, "0af0")==0) && (strcmp(pid, "7401")==0))
		{
			nvram_set("d3g", "Option-iCON-401");
			write_3g_conf(fp, SN_Option_iCON401, 1, vid, pid);
		}
		else if ((strcmp(vid, "0af0")==0) && (strcmp(pid, "7501")==0))
		{
			nvram_set("d3g", "Vodafone-K3760");
			write_3g_conf(fp, SN_Vodafone_K3760, 1, vid, pid);
		}
		else if ((strcmp(vid, "0af0")==0) && (strcmp(pid, "d033")==0))
		{
			nvram_set("d3g", "ATT-USBConnect-Quicksilver");
			write_3g_conf(fp, SN_ATT_USBConnect_Quicksilver, 1, vid, pid);
		}
		else if ((strcmp(vid, "12d1")==0) && (strcmp(pid, "1001")==0))
			write_3g_conf(fp, SN_Huawei_E169, 1, vid, pid);
		else if ((strcmp(vid, "12d1")==0) && (strcmp(pid, "1003")==0))
			write_3g_conf(fp, SN_Huawei_E220, 1, vid, pid);
		else if ((strcmp(vid, "12d1")==0) && (strcmp(pid, "1414")==0))
			write_3g_conf(fp, SN_Huawei_E180, 1, vid, pid);
		else if ((strcmp(vid, "1033")==0) && (strcmp(pid, "0035")==0))
			write_3g_conf(fp, SN_Huawei_E630, 1, vid, pid);
		else if ((strcmp(vid, "12d1")==0) && (strcmp(pid, "1446")==0))	// E1550, E1612, E1690
			write_3g_conf(fp, SN_Huawei_E270, 1, vid, pid);
		else if ((strcmp(vid, "19d2")==0) && (strcmp(pid, "2000")==0))	// also ZTE622, 628, 626, 6535-Z, K3520-Z, K3565, ONDA-MT503HS, ONDA-MT505UP
		{
			nvram_set("d3g", "ZTE-MF626");
			write_3g_conf(fp, SN_ZTE_MF626, 1, vid, pid);
		}
		else if ((strcmp(vid, "19d2")==0) && (strcmp(pid, "fff5")==0))
			write_3g_conf(fp, SN_ZTE_AC8710, 1, vid, pid);	// 2710
		else if ((strcmp(vid, "1410")==0) && (strcmp(pid, "5010")==0))	// U727
			write_3g_conf(fp, SN_Novatel_Wireless_Ovation_MC950D, 1, vid, pid);
		else if ((strcmp(vid, "1410")==0) && (strcmp(pid, "5020")==0))
			write_3g_conf(fp, SN_Novatel_MC990D, 1, vid, pid);
		else if ((strcmp(vid, "1410")==0) && (strcmp(pid, "5030")==0))
			write_3g_conf(fp, SN_Novatel_U760, 1, vid, pid);
		else if ((strcmp(vid, "1c9e")==0) && (strcmp(pid, "1001")==0))
			write_3g_conf(fp, SN_Alcatel_X020, 1, vid, pid);
		else if ((strcmp(vid, "1bbb")==0) && (strcmp(pid, "f000")==0))
			write_3g_conf(fp, SN_Alcatel_X200, 1, vid, pid);
		//else if ((strcmp(vid, "1a8d")==0) && (strcmp(pid, "1000")==0))
		//	write_3g_conf(fp, SN_BandLuxe_C120, 1, vid, pid);
		else if ((strcmp(vid, "1a8d")==0) && (strcmp(pid, "1000")==0))
		{
			nvram_set("d3g", "BandLuxe-C170");
		}
		else if ((strcmp(vid, "1dd6")==0) && (strcmp(pid, "1000")==0))
			write_3g_conf(fp, SN_Solomon_S3Gm660, 1, vid, pid);
		else if ((strcmp(vid, "16d8")==0) && (strcmp(pid, "6803")==0))
			write_3g_conf(fp, SN_C_motechD50, 1, vid, pid);
		else if ((strcmp(vid, "16d8")==0) && (strcmp(pid, "f000")==0))
			write_3g_conf(fp, SN_C_motech_CGU628, 1, vid, pid);
		else if ((strcmp(vid, "0930")==0) && (strcmp(pid, "0d46")==0))
			write_3g_conf(fp, SN_Toshiba_G450, 1, vid, pid);
		else if ((strcmp(vid, "106c")==0) && (strcmp(pid, "3b03")==0))
			write_3g_conf(fp, SN_UTStarcom_UM175, 1, vid, pid);
		else if ((strcmp(vid, "1ab7")==0) && (strcmp(pid, "5700")==0))
			write_3g_conf(fp, SN_Hummer_DTM5731, 1, vid, pid);
		else if ((strcmp(vid, "1199")==0) && (strcmp(pid, "0fff")==0))	// Sierra881U
			write_3g_conf(fp, SN_Sierra_Wireless_Compass597, 1, vid, pid);
		else if ((strcmp(vid, "0fce")==0) && (strcmp(pid, "d0e1")==0))
			write_3g_conf(fp, SN_Sony_Ericsson_MD400, 1, vid, pid);
		else if ((strcmp(vid, "1004")==0) && (strcmp(pid, "1000")==0))
			write_3g_conf(fp, SN_LG_LDU_1900D, 1, vid, pid);
		else if ((strcmp(vid, "1c9e")==0) && (strcmp(pid, "f000")==0))	// ST-Mobile, MobiData MBD-200HU
			write_3g_conf(fp, SN_MobiData_MBD_200HU, 1, vid, pid);
		else if ((strcmp(vid, "1c9e")==0) && (strcmp(pid, "9200")==0))
			write_3g_conf(fp, SN_MyWave_SW006, 1, vid, pid);
		else if ((strcmp(vid, "1f28")==0) && (strcmp(pid, "0021")==0))
			write_3g_conf(fp, SN_Cricket_A600, 1, vid, pid);
		else if ((strcmp(vid, "1b7d")==0) && (strcmp(pid, "0700")==0))
			write_3g_conf(fp, SN_EpiValley_SEC7089, 1, vid, pid);
		else if ((strcmp(vid, "04e8")==0) && (strcmp(pid, "f000")==0))
			write_3g_conf(fp, SN_Samsung_U209, 1, vid, pid);
		else if ((strcmp(vid, "05c6")==0) && (strcmp(pid, "2001")==0))
			write_3g_conf(fp, SN_D_Link_DWM162_U5, 1, vid, pid);
		else if ((strcmp(vid, "1410")==0) && (strcmp(pid, "5031")==0))
			write_3g_conf(fp, SN_Novatel_MC760, 1, vid, pid);
		else if ((strcmp(vid, "19d2")==0) && (strcmp(pid, "0053")==0))
			write_3g_conf(fp, SN_ZTE_MF110, 1, vid, pid);
		else if ((strcmp(vid, "0471")==0) && (strcmp(pid, "1237")==0))	// HuaXing E600
			write_3g_conf(fp, SN_Philips_TalkTalk, 1, vid, pid);
		else if ((strcmp(vid, "16d8")==0) && (strcmp(pid, "700a")==0))
			write_3g_conf(fp, SN_C_motech_CHU_629S, 1, vid, pid);
		else if ((strcmp(vid, "1076")==0) && (strcmp(pid, "7f40")==0))
			write_3g_conf(fp, SN_Sagem9520, 1, vid, pid);
		else if ((strcmp(vid, "0421")==0) && (strcmp(pid, "0610")==0))
			write_3g_conf(fp, SN_Nokia_CS15, 1, vid, pid);
		else if ((strcmp(vid, "12d1")==0) && (strcmp(pid, "1520")==0))
			write_3g_conf(fp, SN_Huawei_K3765, 1, vid, pid);
		else if ((strcmp(vid, "12d1")==0) && (strcmp(pid, "1521")==0))
			write_3g_conf(fp, SN_Huawei_K4505, 1, vid, pid);
		else if ((strcmp(vid, "0471")==0) && (strcmp(pid, "1210")==0))
			write_3g_conf(fp, SN_Vodafone_MD950, 1, vid, pid);
		else if ((strcmp(vid, "05c6")==0) && (strcmp(pid, "f000")==0))
			write_3g_conf(fp, SN_Siptune_LM75, 1, vid, pid);
		else
			write_3g_conf(fp, UNKNOWNDEV, 1, vid, pid);
	}
	else	/* manaul setting */
	{
		nvram_set("d3g", nvram_safe_get("Dev3G"));

		if (strcmp(nvram_safe_get("Dev3G"), "MU-Q101") == 0) {					// on list
			write_3g_conf(fp, SN_MU_Q101, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ASUS-T500") == 0) {				// on list
			write_3g_conf(fp, UNKNOWNDEV, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "OPTION-ICON225") == 0) {			// on list
			write_3g_conf(fp, SN_OPTION_ICON225, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeSurfer-Icon") == 0) {
			write_3g_conf(fp, SN_Option_GlobeSurfer_Icon, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeSurfer-Icon-7.2") == 0) {
			write_3g_conf(fp, SN_Option_GlobeSurfer_Icon72, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeTrotter-GT-MAX-3.6") == 0) {
			write_3g_conf(fp, SN_Option_GlobeTrotter_GT_MAX36, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeTrotter-GT-MAX-7.2") == 0) {
			write_3g_conf(fp, SN_Option_GlobeTrotter_GT_MAX72, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeTrotter-EXPRESS-7.2") == 0) {
			write_3g_conf(fp, SN_Option_GlobeTrotter_EXPRESS72, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-iCON-210") == 0) {
			write_3g_conf(fp, SN_Option_iCON210, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-GlobeTrotter-HSUPA-Modem") == 0) {
			write_3g_conf(fp, SN_Option_GlobeTrotter_HSUPA_Modem, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Option-iCON-401") == 0) {
			write_3g_conf(fp, SN_Option_iCON401, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Vodafone-K3760") == 0) {
			write_3g_conf(fp, SN_Vodafone_K3760, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ATT-USBConnect-Quicksilver") == 0) {
			write_3g_conf(fp, SN_ATT_USBConnect_Quicksilver, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "HUAWEI-E160G") == 0) {			// on list
			write_3g_conf(fp, UNKNOWNDEV, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "HUAWEI-E169") == 0) {			// on list
			write_3g_conf(fp, SN_Huawei_E169, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "HUAWEI-E176") == 0) {			// on list
			write_3g_conf(fp, UNKNOWNDEV, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E180") == 0) {			// on list
			write_3g_conf(fp, SN_Huawei_E180, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "HUAWEI-E220") == 0) {			// on list
			write_3g_conf(fp, SN_Huawei_E220, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E630") == 0) {
			write_3g_conf(fp, SN_Huawei_E630, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E270") == 0) {
			write_3g_conf(fp, SN_Huawei_E270, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E1550") == 0) {
			write_3g_conf(fp, SN_Huawei_E1550, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E161") == 0) {
			write_3g_conf(fp, SN_Huawei_E1612, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E1612") == 0) {
			write_3g_conf(fp, SN_Huawei_E1612, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-E1690") == 0) {
			write_3g_conf(fp, SN_Huawei_E1690, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-K3765") == 0) {
			write_3g_conf(fp, SN_Huawei_K3765, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Huawei-K4505") == 0) {
			write_3g_conf(fp, SN_Huawei_K4505, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-MF620") == 0) {
			write_3g_conf(fp, SN_ZTE_MF620, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-MF622") == 0) {
			write_3g_conf(fp, SN_ZTE_MF622, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-MF628") == 0) {
			write_3g_conf(fp, SN_ZTE_MF628, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-MF626") == 0) {
			write_3g_conf(fp, SN_ZTE_MF626, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-AC8710") == 0) {
			write_3g_conf(fp, SN_ZTE_AC8710, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-AC2710") == 0) {
			write_3g_conf(fp, SN_ZTE_AC2710, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-6535-Z") == 0) {
			write_3g_conf(fp, SN_ZTE6535_Z, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-K3520-Z") == 0) {
			write_3g_conf(fp, SN_ZTE_K3520_Z, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-MF110") == 0) {
			write_3g_conf(fp, SN_ZTE_MF110, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ZTE-K3565") == 0) {
			write_3g_conf(fp, SN_ZTE_K3565, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ONDA-MT503HS") == 0) {
			write_3g_conf(fp, SN_ONDA_MT503HS, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ONDA-MT505UP") == 0) {
			write_3g_conf(fp, SN_ONDA_MT505UP, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Novatel-Wireless-Ovation-MC950D-HSUPA") == 0) {
			write_3g_conf(fp, SN_Novatel_Wireless_Ovation_MC950D, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Novatel-U727") == 0) {
			write_3g_conf(fp, SN_Novatel_U727, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Novatel-MC990D") == 0) {
			write_3g_conf(fp, SN_Novatel_MC990D, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Novatel-U760") == 0) {
			write_3g_conf(fp, SN_Novatel_U760, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Alcatel-X020") == 0) {
			write_3g_conf(fp, SN_Alcatel_X020, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Alcatel-X200") == 0) {
			write_3g_conf(fp, SN_Alcatel_X200, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "AnyDATA-ADU-500A") == 0) {
			write_3g_conf(fp, SN_AnyDATA_ADU_500A, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "BandLuxe-C120") == 0) {			// on list
			write_3g_conf(fp, SN_BandLuxe_C120, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "BandLuxe-C170") == 0) {			// on list
			nvram_set("d3g", "BandLuxe-C170");	// band270
		} else if (strcmp(nvram_safe_get("Dev3G"), "Solomon-S3Gm-660") == 0) {
			write_3g_conf(fp, SN_Solomon_S3Gm660, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "C-motechD-50") == 0) {
			write_3g_conf(fp, SN_C_motechD50, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "C-motech-CGU-628") == 0) {
			write_3g_conf(fp, SN_C_motech_CGU628, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Toshiba-G450") == 0) {
			write_3g_conf(fp, SN_Toshiba_G450, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "UTStarcom-UM175") == 0) {
			write_3g_conf(fp, SN_UTStarcom_UM175, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Hummer-DTM5731") == 0) {
			write_3g_conf(fp, SN_Hummer_DTM5731, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "A-Link-3GU") == 0) {
			write_3g_conf(fp, SN_A_Link_3GU, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Sierra-Wireless-Compass-597") == 0) {
			write_3g_conf(fp, SN_Sierra_Wireless_Compass597, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Sierra-881U") == 0) {
			write_3g_conf(fp, SN_Sierra881U, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Sony-Ericsson-MD400") == 0) {
			write_3g_conf(fp, SN_Sony_Ericsson_MD400, 0, vid, pid);
		//} else if (strcmp(nvram_safe_get("Dev3G"), "Sony-Ericsson-W910i") == 0) {		// on list
		//	write_3g_conf(fp, UNKNOWNDEV, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "LG-LDU-1900D") == 0) {
			write_3g_conf(fp, SN_LG_LDU_1900D, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Samsung-SGH-Z810") == 0) {
			write_3g_conf(fp, SN_Samsung_SGH_Z810, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "MobiData-MBD-200HU") == 0) {
			write_3g_conf(fp, SN_MobiData_MBD_200HU, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "ST-Mobile") == 0) {
			write_3g_conf(fp, SN_ST_Mobile, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "MyWave-SW006") == 0) {
			write_3g_conf(fp, SN_MyWave_SW006, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Cricket-A600") == 0) {
			write_3g_conf(fp, SN_Cricket_A600, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "EpiValley-SEC-7089") == 0) {
			write_3g_conf(fp, SN_EpiValley_SEC7089, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Samsung-U209") == 0) {
			write_3g_conf(fp, SN_Samsung_U209, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "D-Link-DWM-162-U5") == 0) {
			write_3g_conf(fp, SN_D_Link_DWM162_U5, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Novatel-MC760") == 0) {
			write_3g_conf(fp, SN_Novatel_MC760, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Philips-TalkTalk") == 0) {
			write_3g_conf(fp, SN_Philips_TalkTalk, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "HuaXing-E600") == 0) {
			write_3g_conf(fp, SN_HuaXing_E600, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "C-motech-CHU-629S") == 0) {
			write_3g_conf(fp, SN_C_motech_CHU_629S, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Sagem-9520") == 0) {
			write_3g_conf(fp, SN_Sagem9520, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Nokia-CS-15") == 0) {
			write_3g_conf(fp, SN_Nokia_CS15, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Vodafone-MD950") == 0) {
			write_3g_conf(fp, SN_Vodafone_MD950, 0, vid, pid);
		} else if (strcmp(nvram_safe_get("Dev3G"), "Siptune-LM-75") == 0) {
			write_3g_conf(fp, SN_Siptune_LM75, 0, vid, pid);
		} else{
			fprintf(fp, "DefaultVendor=  0x%s;\n", vid);
			fprintf(fp, "DefaultProduct=  0x%s\n\n", pid);
		}
	}
	fclose(fp);

	return 0;
}

int rc_start_3g()
{
	unsigned int usb_dev_class_num;
	char product[20], productID[10], vendorID[10];

	memset(productID, 0, sizeof(productID));
	memset(vendorID, 0, sizeof(vendorID));
	memset(product, 0, sizeof(product));

	get_dev_info(&usb_dev_class_num, product, vendorID, productID);
	if ((strlen(vendorID) > 0) && (strlen(productID) > 0))
	{
		dbg("get vendor=%s, product=%s\n", vendorID, productID);     // tmp test
		logmessage("USB device", "usb device %s(%s/%s) plugged", nvram_safe_get("usb_Product"), vendorID, productID);
	}
	else
	{
		dbg("rc 3g: invalid vid/pid\n");	// tmp test
		logmessage("USB device", "invalid usb device vid/pid");
		nvram_set("usb_dev_state", "off");
		track_set("203");
		return -1;
	}

	nvram_set("usb_dev_state", "on");
	nvram_set("usb_path1", "HSDPA");

	init_3g_param(vendorID, productID);

	start_3g_process();
	return 0;
}
#endif

/*
int 
do_eject(char *dev_name)
{
	int ejectfd;
	int status;
	struct sdata {
		int  inlen;
		int  outlen;
		char cmd[256];
	} scsi_cmd;

	dbg("Ejecting SCSI(%s) of CDROM ...", dev_name);

	if ((ejectfd = open(dev_name, O_RDONLY)) < 0) {
		dbg("there wasn't any CD-ROM device!\n");
		return -1;
	}
	dbg("there was a CD-ROM device!\n");

	scsi_cmd.inlen  = 0;
	scsi_cmd.outlen = 0;
	scsi_cmd.cmd[0] = START_STOP;
	scsi_cmd.cmd[1] = 0;
	scsi_cmd.cmd[2] = 0;
	scsi_cmd.cmd[3] = 0;
	scsi_cmd.cmd[4] = 1;
	scsi_cmd.cmd[5] = 0;

	status = ioctl(ejectfd, SCSI_IOCTL_SEND_COMMAND, (void *)&scsi_cmd);
	if (status < 0) {
		dbg("Failed to send first START_STOP!\n");
		perror("send first START_STOP");
		close(ejectfd);
		return -1;
	}

	scsi_cmd.inlen  = 0;
	scsi_cmd.outlen = 0;
	scsi_cmd.cmd[0] = START_STOP;
	scsi_cmd.cmd[1] = 0;
	scsi_cmd.cmd[2] = 0;
	scsi_cmd.cmd[3] = 0;
	scsi_cmd.cmd[4] = 2;
	scsi_cmd.cmd[5] = 0;
	status = ioctl(ejectfd, SCSI_IOCTL_SEND_COMMAND, (void *)&scsi_cmd);
	if (status != 0) {
		dbg("Failed to send second START_STOP!\n");
		perror("send second START_STOP");
		close(ejectfd);
		return 0;
	}

	close(ejectfd);
	dbg("end do eject\n");	// tmp test
	return 0;
}
*/

int 
hotplug_usb_test()
{
	dbg("enter hotplug_usb_test()...\n");

	char *action, *devpath, *subsystem, *device, *usbdevice_path, *product, *type, *interface;
	FILE *fp;

	action = getenv("ACTION");
	devpath = getenv("DEVPATH");
	subsystem = getenv("SUBSYSTEM");
	device = getenv("DEVICE");
	usbdevice_path = getenv("USBDEVICE_PATH");
	product = getenv("PRODUCT");
	type = getenv("TYPE");
	interface = getenv("INTERFACE");

	fp = fopen("/tmp/hotplug_usb_test", "a");

	if (action)
	{
		dbg("ACTION: %s\n", action);
		if (fp)
			fprintf(fp, "ACTION: %s\n", action);
	}
	if (devpath)
	{
		dbg("DEVPATH: %s\n", devpath);
		if (fp)
			fprintf(fp, "DEVPATH: %s\n", devpath);
	}
	if (subsystem)
	{
		dbg("SUBSYSTEM: %s\n", subsystem);
		if (fp)
			fprintf(fp, "SUBSYSTEM: %s\n", subsystem);
	}
	if (device)
	{
		dbg("DEVICE: %s\n", device);
		if (fp)
			fprintf(fp, "DEVICE: %s\n", device);
	}
	if (usbdevice_path)
	{
		dbg("USBDEVICE_PATH: %s\n", usbdevice_path);
		if (fp)
			fprintf(fp, "USBDEVICE_PATH: %s\n", usbdevice_path);
	}
	if (product)
	{
		dbg("PRODUCT: %s\n", product);
		if (fp)
			fprintf(fp, "PRODUCT: %s\n", product);
	}
	if (type)
	{
		dbg("TYPE: %s\n", type);
		if (fp)
			fprintf(fp, "TYPE: %s\n", type);
	}
	if (interface)
	{
		dbg("INTERFACE: %s\n", interface);
		if (fp)
			fprintf(fp, "INTERFACE: %s\n", interface);
	}

	if (fp)
		fclose(fp);

	return 0;
}

int is_usb_storage_loaded()
{
	DIR *dir_to_open = NULL;

	dir_to_open = opendir("/sys/module/usb_storage");
	if (dir_to_open)
	{
		closedir(dir_to_open);
		return 1;
	}
		return 0;
}

int count_umount_ejected;

int 
hotplug_usb()
{
	dbg("%s()...\n", __FUNCTION__);

	unsigned int usb_dev_class_num, bus_plugged = 0;
	char productID[20], prid[10], veid[10];
//	char *product;
	char *usb_path;
	char usbpath_nvram[16];
	char temp_usbpath_device[16];
	FILE *fp;

	count_umount_ejected = 0;

//	if (nvram_match("asus_mfg", "1"))
//	{
//		nvram_set("usb_device", "1");
//		return 0;
//	}
/*
	product = getenv("PRODUCT");
	usb_path = getenv("USBDEVICE_PATH");

	if (strcmp(usb_path, "1") && strcmp(usb_path, "2"))
		usb_path = NULL;

	if (usb_path) {   // tmp add
		nvram_set("usb_path", usb_path);

		memset(usbpath_nvram, 0, 16);
		sprintf(usbpath_nvram, "usb_path%s", usb_path);

		memset(temp_usbpath_device, 0, 16);
		strcpy(temp_usbpath_device, nvram_safe_get(usbpath_nvram));
	}
*/

	parse_proc_bus_usb_devices();

	memset(productID, 0, sizeof(productID));

//	bus_plugged =  get_dev_info(&usb_dev_class_num, productID, veid, prid);
//	if (bus_plugged)
//	dbg("hotplug_usb: bus plugged is %d, pID is %s, class_num is %d\n", bus_plugged, productID, usb_dev_class_num);  // tmp test

	memset(usb_path1_old, 0x0, 16);
	memset(usb_path2_old, 0x0, 16);
	strcpy(usb_path1_old, usb_path1);
	strcpy(usb_path2_old, usb_path2);
	memset(usb_path1, 0x0, 16);
	memset(usb_path2, 0x0, 16);
	strcpy(usb_path1, nvram_safe_get("usb_path1"));
	strcpy(usb_path2, nvram_safe_get("usb_path2"));

	dbg("usb_path1: %s\n", usb_path1);
	dbg("usb_path1_old: %s\n", usb_path1_old);
	dbg("usb_path2: %s\n", usb_path2);
	dbg("usb_path2_old: %s\n", usb_path2_old);
	

	if (!strcmp(usb_path1, "storage") && !strcmp(usb_path1_old, ""))
	{
		bus_plugged = 1;
	}
	else if (!strcmp(usb_path2, "storage") && !strcmp(usb_path2_old, ""))
	{
		bus_plugged = 1;
	}
	else if (!strcmp(usb_path1, usb_path1_old) && !strcmp(usb_path2, usb_path2_old))
	{
		bus_plugged = 0;
	}
	else
	{
		bus_plugged = -1;
	}

	/* clean up */
	if (strcmp(usb_path1, "storage") && strcmp(usb_path2, "storage"))
	{
		nvram_set("usb_mnt_first_path", "");
		nvram_set("usb_mnt_first_path_port", "0");

		nvram_set("apps_status_checked", "1");	// it means need to check
		nvram_set("apps_comp", "0");
		nvram_set("apps_disk_free", "0");
	}

	if (bus_plugged == 1)
	{
MANUAL_HOTPLUG_USB_MASS:
		dbg("!!! %s(): action ADD !!!\n", __FUNCTION__);

//		logmessage("USB storage", "\"%s\" was plugged in.", productID);
		logmessage("USB storage", "plugged in");

//		dbg("service_ex: start excuting usb_hotplug process, type: %d\n", usb_dev_class_num);	// tmp test
//		if (usb_dev_class_num == USB_CLS_MASS_STORAGE)
		{
			dbg("we got storage dev\n"); // tmp test

			pre_chk_mount();
			sleep(1);
			chk_partitions(USB_PLUG_ON);
			hotplug_usb_mass(productID);
/*
			test_write_result = test_write_firstP();
			if (test_write_result > 0)
				;
			else
			{
				dbg("[usb test write]: disk error\n");	// tmp test
				logmessage("USB storage", "Disk error with unknown reason\n");
			}
*/
		}
	}
	else if (bus_plugged == 0)
	{
		dbg("!!! %s(): IGNORE !!!\n", __FUNCTION__);
		return 0;
	}
	else
	{
		dbg("!!! %s(): action REMOVE !!!\n", __FUNCTION__);
		logmessage("USB storage", "removed");

		if (	strcmp(usb_path1, "storage") &&
			strcmp(usb_path2, "storage")/* &&
			!count_sddev_mountpoint() &&
			!count_sddev_partition()*/	)
		{
			dbg("!!! %s(): action REMOVE case 1 !!!\n", __FUNCTION__);

			/* 0708 check: umount */
			nvram_set("apps_comp", "0");
			nvram_set("apps_disk_free", "0");

			nvram_set("mnt_type", "");
			/* 0708 check end */

			remove_usb_mass("1");
			remove_usb_mass("2");

			umount_sddev_all();

			int try_count = 0;
			while ((is_usb_storage_loaded()) && (try_count++ < 5))
			{
				usleep(500 * 1000);
				system("rmmod usb-storage");
			}

			if (!is_usb_storage_loaded())
			{
				usleep(500 * 1000);
				system("insmod -q usb-storage");
			}

			if (swap_check())
			{
				dbg("failed to swap off !!!\n");
				dbg("system is going to reboot !!!\n");
				kill(1, SIGTERM);	// reboot
			}
		}
		else if (strcmp(usb_path1, "storage") && !strcmp(usb_path1_old, "storage"))
		{
			dbg("!!! %s(): action REMOVE case 2 !!!\n", __FUNCTION__);
//			remove_usb_mass("1");
			remove_usb_mass(NULL);

			if (count_umount_ejected)
			{
				count_umount_ejected = 0;
				restart_apps();
			}
		}
		else if (strcmp(usb_path2, "storage") && !strcmp(usb_path2_old, "storage"))
		{
			dbg("!!! %s(): action REMOVE case 3 !!!\n", __FUNCTION__);
//			remove_usb_mass("2");
			remove_usb_mass(NULL);

			if (count_umount_ejected)
			{
				count_umount_ejected = 0;
				restart_apps();
			}
		}
		else
		{
			dbg("!!! %s(): action REMOVE case 4 !!!\n", __FUNCTION__);
			remove_usb_mass(NULL);

			if (count_umount_ejected)
			{
				count_umount_ejected = 0;
				restart_apps();
			}
		}

		if (!is_apps_running())
		{
			if ((fp=fopen("/proc/sys/net/nf_conntrack_max", "w+")))
			{
				if (nvram_get("misc_conntrack_x") == NULL)
					fputs(MAX_CONNTRACK_DM, fp);
				else
				{
//					dbg("\nrestore nf_conntract_max...\n\n");
					fputs(nvram_safe_get("misc_conntrack_x"), fp);
				}
				fclose(fp);
			}
		}
	}

	return 0;
}

#endif	

int stop_service_type_99 = 0;

/* stop necessary services for firmware upgrade */	
/* stopservice: for firmware upgarde */
/* stopservice 1: for button setup   */
void
stop_service_main(int type)
{
	dbg("stop service type: %d\n", type);

	if (type==1)
	{
		stop_usb();
		stop_upnp();
		stop_dns();
		stop_httpd();
		if (pids("udhcpc"))
			system("killall -SIGTERM udhcpc");
	}
	else
	{
		if (type==99)
		{
			nvram_set("reboot", "1");
			stop_service_type_99 = 1;
			stop_misc_no_watchdog();
		}
		else
			stop_misc();

		stop_logger();
		stop_usb();

		stop_wanduck();
		stop_upnp();
		stop_dhcpd();
//		stop_dns();
		stop_ots();
		stop_networkmap();
//		stop_telnetd();
		if (pids("igmpproxy"))
			system("killall igmpproxy");
	}
}

int service_handle(void)
{
	char *service;
	char tmp[100], *str;
	int pid;
	char *ping_argv[] = { "ping", "8.8.8.8", "-c", "3", NULL};
	FILE *fp;
	char pid_buf[32];

	service = nvram_get("rc_service");
#if 0
	if (!service)
	{
		dbg("[rc] chk sys restart 1\n");	// tmp test

		stop_usb();

		kill(1, SIGHUP);
	}
#endif
	if (strstr(service,"wan_disconnect")!=NULL)
	{
		logmessage("wan", "disconnected manually");
		dbg("wan disconnect manually\n");	// tmp test

#ifdef RTCONFIG_USB_MODEM
		if (get_usb_modem_state()){
			stop_wan_ppp();
		}
		else
#endif
		if (nvram_match("wan0_proto", "dhcp") ||
#ifdef CDMA_REMOVE
		    nvram_match("wan0_proto", "cdma") ||
#endif
			nvram_match("wan0_proto", "bigpond"))
		{	/* dhcp */
			snprintf(tmp, sizeof(tmp), "/var/run/udhcpc%d.pid", 0);
			if ((str = file2str(tmp))) {
				pid = atoi(str);
				free(str);
				logmessage("service_handle", "perform DHCP release");
				kill(pid, SIGUSR2);
			}
		}
		else if (
		nvram_match("wan0_proto", "pptp")  ||
		nvram_match("wan0_proto", "pppoe") ||
		nvram_match("wan0_proto", "l2tp")
		)
		{	/* pptp, l2tp, pppoe */
			dbg("stop wan ppp manually\n");	// tmp test
			stop_wan_ppp();
		}
		else 	/* static */
		{			
			dbg("services stop wan2 \n");	// tmp test
			stop_wan2();
			update_wan_status(0);
			//sleep(2);

			if (nvram_match("wan0_proto", "static"))
			{
				system("ifconfig eth3 0.0.0.0");
			}
#ifndef RTCONFIG_USB_MODEM
			else if (nvram_match("wan0_proto", "3g"))
			{
				nvram_set("wan0_ipaddr", "");
			}
#endif
		}
	}
	else if (strstr(service,"wan_connect")!=NULL)
	{
		logmessage("wan", "connected manually");
		dbg("wan connect manually\n");	// tmp test

#ifdef RTCONFIG_USB_MODEM
		if (get_usb_modem_state()){
			restart_wan_ppp();
		}
		else
#endif
		if (nvram_match("wan0_proto", "dhcp") ||
#ifdef CDMA_REMOVE
		    nvram_match("wan0_proto", "cdma") ||
#endif
			nvram_match("wan0_proto", "bigpond"))
		{	/* dhcp */
			snprintf(tmp, sizeof(tmp), "/var/run/udhcpc%d.pid", 0);
			if ((str = file2str(tmp))) {
				pid = atoi(str);
				free(str);
				logmessage("service_handle", "perform DHCP renew");
				kill(pid, SIGUSR1);
			}
		}
		else if (
		nvram_match("wan0_proto", "pptp")  ||
		nvram_match("wan0_proto", "pppoe") ||
		nvram_match("wan0_proto", "l2tp")
		)
		{	/* pptp, l2tp, pppoe */
			dbg("start wan ppp manually\n");	// tmp test
			restart_wan_ppp();
		}
		else 
		{
			// pppoe or ppptp, check if /tmp/ppp exist
			if (!nvram_match("wan0_proto", "static") && !nvram_match("wan0_proto", "3g") && (fp=fopen("/tmp/ppp/ip-up", "r"))!=NULL)
			{	/* none */
				dbg("chk none\n");	// tmp test
				fclose(fp);
				_eval(ping_argv, NULL, 0, &pid);
			}
#ifndef RTCONFIG_USB_MODEM
			else if (nvram_match("wan0_proto", "3g"))
			{
				dbg("start 3g manually\n");	// tmp test
				rc_start_3g();
			}
#endif
			else
			{	/* static */
				dbg("chk staic\n");	// tmp test
				stop_wan();
				unlink("/tmp/ppp/link.ppp0");
				unlink("/tmp/ppp/options.wan0");

				sleep(1);

				start_wan();

				sleep(2);

				_eval(ping_argv, NULL, 0, &pid);
			}
		}
	}
#ifdef ASUS_DDNS //2007.03.26 Yau add for asus ddns
	else if (strstr(service,"ddns_hostname_check"))
	{
		char wan_ifname[16];

		if (nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "pptp") || nvram_match("wan_proto", "l2tp")) //2008.10 magic add l2tp
		{
			strcpy(wan_ifname, nvram_safe_get("wan0_pppoe_ifname"));
		}
		else
		{
			strcpy(wan_ifname, nvram_safe_get("wan0_ifname"));
		}

		//Execute ez-ipupdate then die.
		if (pids("ez-ipupdate"))
		{
			system("killall -SIGINT ez-ipupdate");
			sleep(1);
		}

		doSystem("ez-ipupdate -h %s -s ns1.asuscomm.com -S dyndns -i %s -A 1", nvram_safe_get("ddns_hostname_x"), wan_ifname);

		nvram_set("ddns_enable_x", nvram_safe_get("bak_ddns_enable_x"));
		nvram_set("ddns_wildcard_x", nvram_safe_get("bak_ddns_wildcard_x"));
		nvram_commit_safe();
	}
	else if (strstr(service,"restart_upnp") && nvram_match("router_disable", "0"))
	{

		if (nvram_match("upnp_enable", "1") && nvram_match("ntp_restart_upnp", "0"))
		{
			if (!nvram_match("ntp_ready", "0"))
			{
				nvram_set("ntp_restart_upnp", "1");
			}

			if (	nvram_match("ntp_ready", "1") ||
				(nvram_match("ntp_ready", "2") && nvram_match("upnp_started", "0")) )
			{
				dbg("[rc] ntp restart upnp...\n");
				stop_upnp();
				start_upnp();
			}
		}
	}
#endif
	nvram_unset("rc_service");
	return 0;
}

#ifdef AUDIO_SUPPORT
int hotplug_usb_audio(char *product)
{					
	char *wave_argv[]={"waveserver", NULL};
	pid_t pid;

	if (strlen(product)==0) return;
	_eval(wave_argv, ">/dev/null", 0, NULL);
}

int remove_usb_audio(char *product)
{
	if (pids("waveserver"))
		system("killall waveserver");
}

int
start_audio(void)
{
	char *wave_argv[] = {"waveservermain", NULL};
	pid_t pid;

	_eval(wave_argv, NULL, 0, &pid);
	return 0;
}

void
stop_audio(void)
{
	if (pids("waveserver"))
		system("killall waveserver");
}
#endif
#endif

#if 0
#ifdef GUEST_ACCOUNT
int
start_dhcpd_guest(void)
{
	FILE *fp;
	char *dhcpd_argv[] = {"udhcpd", "/tmp/udhcpd1.conf", NULL, NULL};
	char *slease = "/tmp/udhcpd-br1.sleases";
	pid_t pid;

	if (nvram_match("router_disable", "1") || !nvram_match("wl_guest_ENABLE", "1") || !nvram_match("lan1_proto", "dhcp") || nvram_match("mssid_ENABLE", "1"))
		return 0;

	dprintf("%s %s %s %s\n",
		nvram_safe_get("lan1_ifname"),
		nvram_safe_get("dhcp1_start"),
		nvram_safe_get("dhcp1_end"),
		nvram_safe_get("lan1_lease"));

	if (!(fp = fopen("/tmp/udhcpd-br1.leases", "a"))) {
		perror("/tmp/udhcpd-br1.leases");
		return errno;
	}
	fclose(fp);

	/* Write configuration file based on current information */
	if (!(fp = fopen("/tmp/udhcpd1.conf", "w"))) {
		perror("/tmp/udhcpd1.conf");
		return errno;
	}
	
	fprintf(fp, "pidfile /var/run/udhcpd-br1.pid\n");
	fprintf(fp, "start %s\n", nvram_safe_get("dhcp1_start"));
	fprintf(fp, "end %s\n", nvram_safe_get("dhcp1_end"));
	fprintf(fp, "interface %s\n", nvram_safe_get("lan1_ifname"));
	fprintf(fp, "remaining yes\n");
	fprintf(fp, "lease_file /tmp/udhcpd-br1.leases\n");
	fprintf(fp, "option subnet %s\n", nvram_safe_get("lan1_netmask"));
	fprintf(fp, "option router %s\n", nvram_safe_get("lan1_ipaddr"));	
	
	if (!nvram_match("dhcp_dns1_x", ""))		
		fprintf(fp, "option dns %s\n", nvram_safe_get("dhcp_dns1_x"));		
	fprintf(fp, "option dns %s\n", nvram_safe_get("lan1_ipaddr"));
	fprintf(fp, "option lease %s\n", nvram_safe_get("lan1_lease"));

	if (!nvram_match("dhcp_wins_x", ""))		
		fprintf(fp, "option wins %s\n", nvram_safe_get("dhcp_wins_x"));		
	if (!nvram_match("lan_domain", ""))
		fprintf(fp, "option domain %s\n", nvram_safe_get("lan_domain"));
	fclose(fp);

	dhcpd_argv[2] = NULL;
	_eval(dhcpd_argv, NULL, 0, &pid);


	dprintf("done\n");
	return 0;
}

void
stop_dhcpd_guest(void)
{
	char sigusr1[] = "-XX";

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
		kill_pidfile_s("/var/run/udhcpd.pid", SIGTERM);
}
#endif
#endif	// #if 0

int
fill_smbpasswd_input_file(const char *passwd)
{
	FILE *fp;
	
	unlink("/tmp/smbpasswd");
	fp = fopen("/tmp/smbpasswd", "w");
	
	if (fp && passwd)
	{
		fprintf(fp,"%s\n", passwd);
		fprintf(fp,"%s\n", passwd);
		fclose(fp);
		
		return 1;
	}
	else
		return 0;
}

int
start_samba(void)
{
	FILE *fp;
	int n=0, sh_num=0, i;
	char tmpuser[40], tmp2[40];
	int ret1=0, ret2=0;
	char cmd[256], tmp_path[128];

	if (nvram_match("enable_samba", "0")) return 0;

	stop_samba(); // 2007.11 James. for new samba mode

//	unlink("/tmp/samba/lib/smb.conf");
	unlink("/etc/smb.conf");
//	unlink("/tmp/samba/private/smbpasswd");
	unlink("/etc/smbpasswd");
	unlink("/etc/passwd");
	unlink("/etc/group");

//	symlink("/etc/passwd", "/tmp/passwd");
//	symlink("/etc/group", "/tmp/group");

/* write samba configure file*/
	system("/sbin/write_smb_conf");

/* write /etc/passwd */
	fp = fopen("/etc/passwd", "w");
	fprintf(fp, "nobody:x:99:99::/:\n");
	sh_num = atoi(nvram_safe_get("acc_num"));
	dbg("sh num is %d\n", sh_num);	// tmp test
	memset(tmpuser, 0, sizeof(tmpuser));
	for (i=0, n=500; i<sh_num; i++, n++)
	{
		sprintf(tmpuser, "acc_username%d", i);
		fprintf(fp, "%s:x:%d:%d:::\n", nvram_safe_get(tmpuser), n, n);
	}
	fclose(fp);

/* write /etc/group  */
	fp = fopen("/etc/group", "w");
	fprintf(fp, "nobody:x:99:\n");
	sh_num = atoi(nvram_safe_get("acc_num"));
	memset(tmpuser, 0, sizeof(tmpuser));
	for (i=0, n=500; i<sh_num; i++, n++)
	{
		sprintf(tmpuser, "acc_username%d", i);
		fprintf(fp, "%s:x:%d:\n", nvram_safe_get(tmpuser), n);
	}
	fclose(fp);


	/* change password for telnetd */
	chpass(nvram_safe_get("http_username"), nvram_safe_get("http_passwd"));

/* write smbpasswd  */
//	system("/usr/local/samba/bin/smbpasswd -a nobody \"\"");
//	system("smbpasswd -a nobody \"\"");
	system("smbpasswd -a nobody -n");

	sh_num = atoi(nvram_safe_get("acc_num"));
	memset(tmpuser, 0, sizeof(tmpuser));
	memset(tmp2, 0, sizeof(tmp2));
	for (i=0; i<sh_num; i++)
	{
		sprintf(tmpuser, "acc_username%d", i);
		sprintf(tmp2, "acc_password%d", i);
		sprintf(tmpuser, nvram_safe_get(tmpuser));
		sprintf(tmp2, nvram_safe_get(tmp2));
//		doSystem("/usr/local/samba/bin/smbpasswd -a %s %s", tmpuser, tmp2);
//		doSystem("smbpasswd -a %s %s", tmpuser, tmp2);
		fill_smbpasswd_input_file(tmp2);
		sprintf(cmd, "smbpasswd -a %s -s < /tmp/smbpasswd", tmpuser);
		system(cmd);
	}

//	system("/sbin/nmbd -D -s /etc/smb.conf");
//	system("/sbin/smbd -D -s /etc/smb.conf");
	eval("/sbin/nmbd", "-D", "-s", "/etc/smb.conf");
	eval("/sbin/smbd", "-D", "-s", "/etc/smb.conf");

	nvram_set("apps_smb_ex", "1");

	logmessage("Samba Server", "daemon is started");

	return 0;
}

int
check_disk_free(char *res, char *diskpath)
{
	char *sizebuf, *freebuf, *databuf;
	struct statfs fsbuf;

	if (statfs(diskpath, &fsbuf))
	{
		perror("*** check_disk_free: statfs fail!");
		return 2;
	}

	sizebuf = malloc(32);
	freebuf = malloc(32);
	databuf = malloc(64);

	sprintf(sizebuf, "%.1f", (double)((double)((double)fsbuf.f_blocks * fsbuf.f_bsize)/(1024*1024)));
	sprintf(freebuf, "%.1f", (double)((double)((double)fsbuf.f_bfree * fsbuf.f_bsize)/(1024*1024)));
	sprintf(res, "size: %s, free: %s", sizebuf, freebuf);

	if (sizebuf) 
		free(sizebuf);
	if (freebuf) 
		free(freebuf);
	if (databuf) 
		free(databuf);
	if (fsbuf.f_bfree == 0)
		return 0;
	else
		return 1;
}

#ifdef DLM
int
check_disk_free_GE_1G(char *diskpath)
{
	struct statfs fsbuf;
	double free_size, block_size;

	if (statfs(diskpath, &fsbuf))
	{
		perror("*** check_disk_free_GE_1G: statfs fail!");
		return 0;
	}

	free_size = (double)((double)((double)fsbuf.f_bfree * fsbuf.f_bsize)/(1024*1024));
	block_size = (double)((double)((double)fsbuf.f_blocks * fsbuf.f_bsize)/(1024*1024));
	dbg("\nchk disk: free=%fMB, blocksize=%fMB\n", free_size, block_size);	// tmp test

	if ( free_size < (double)128 )
		return 1;
	else if ( block_size > (double)1024 )
		return 2;
	else
		return 3;
}

int
check_disk_free_apps(char *diskpath, int ac_flag)
{
	struct statfs fsbuf;
	double free_size;

	//dbg("chk disk free apps\n");	// tmp test
	if (statfs(diskpath, &fsbuf))
	{
		perror("*** check_disk_free_apps: statfs fail!");
		return 0;
	}

	free_size = (double)((double)((double)fsbuf.f_bfree * fsbuf.f_bsize)/(1024*1024));
	//dbg("free_size = %f\n", free_size);	// tmp test

	if (ac_flag==1)
	{
		if (nvram_match("apps_dl", "0"))
			return 1;

		if ( free_size < (double)1 )
			return 0;
		else
			return 1;
	}
	else
	{
		if ( free_size < (double)5 )
			return 0;
		else
			return 1;
	}
}
#endif

#ifdef DLM
void set_apps_ver()
{
	nvram_set("apps_ver", "1.00");
}

void exec_apps()
{	
	FILE *fp;
	
	char pool[32];
	char share[32];
	char rundl[8];
	char rundms[8];
	char name[32];
	char mymac[32];
	int cap=0;
	int ret=0;

	char EXBIN[32]="/.apps/bin";
	char test_path[128];

	char tmpstr[256];
	char tmpstr2[256];
//	char share_port_from[5];
//	char share_port_to[5];

	umask(0000);

	dbg("\n## exec apps\n");	// tmp test
	memset(test_path, 0, sizeof(test_path));
	strcpy(test_path, "/tmp/harddisk/part0");
	strcpy(tmpstr, EXBIN);
	sprintf(EXBIN, "%s%s", test_path, tmpstr);
	
	if (pids("snarf"))
		system("killall -SIGKILL snarf");
	if (pids("rtorrent"))
		system("killall -SIGKILL rtorrent");
	if (pids("giftd"))
		system("killall -SIGKILL giftd");
	if (pids("dmathined"))
		system("killall -SIGKILL dmathined");

	nvram_set("apps_installed", "0");

	strcpy(pool, nvram_safe_get("apps_pool"));
	strcpy(share, nvram_safe_get("apps_share"));
	strcpy(rundl, nvram_safe_get("apps_dl"));
	strcpy(rundms, nvram_safe_get("apps_dms"));
	if (!nvram_match("computer_name", "") && is_valid_hostname(nvram_safe_get("computer_name")))
		strcpy(name, nvram_safe_get("computer_name"));
	else
		strcpy(name, nvram_safe_get("productid"));
	strcpy(mymac, nvram_safe_get("lan_hwaddr"));
	
//	strcpy(share_port_from, nvram_safe_get("apps_dl_share_port_from"));
//	strcpy(share_port_to, nvram_safe_get("apps_dl_share_port_to"));
	
	fp=fopen("/tmp/.prange", "w");
	if (fp)
	{
//		fprintf(fp,"%s:%s", share_port_from, share_port_to);
		fprintf(fp,"%s:%s", "10001", "10050");
		fclose(fp);
	}

	if (nvram_match("apps_dl", "1") && swap_check())
	{
		sprintf(tmpstr2, "%s/dmex", EXBIN);
		ret=system(tmpstr2);
	}

	if (is_apps_running())
	{
		logmessage("Download Master", "daemon is started");
		nvram_set("apps_installed", "1");
		nvram_set("apps_status_checked", "0");

		if (nvram_match("wan_route_x", "IP_Routed"))
		{
			dbg("rc_restart_firewall...\n");
			rc_restart_firewall();
		}
#if 0
		if ((fp=fopen("/proc/sys/net/nf_conntrack_max", "w+")))
		{
			dbg("\nreset nf_conntract_max...\n\n");
			fputs(MAX_CONNTRACK_DM, fp);
			fclose(fp);
		}
#endif
	}
}

char *if_dircase_exist(const char *dir, const char *subdir);

void init_apps()
{
	char pool[32];
	char share[32];
	char origver[32];
	char target_dir[32];
	char pool_dir[32];
	char EXLIB[128]="/.apps/lib";
	char EXBIN[128]="/.apps/bin";
	char EXUSR[128]="/.apps/usr";
	char EXETC[32];
	char NEWVER[8];
	char tmpstr[256];
	char tmpstr2[256];
	char tmpstr3[256];
	char tmpstr4[256];

	char test_path[128];
	char *p_share;

	umask(0000);

	memset(test_path, 0, sizeof(test_path));
	strcpy(test_path, "/tmp/harddisk/part0");
	nvram_set("apps_pool", "harddisk/part0");
	
	strcpy(tmpstr, EXLIB);
	sprintf(EXLIB, "%s%s", test_path, tmpstr);
	strcpy(tmpstr, EXBIN);
	sprintf(EXBIN, "%s%s", test_path, tmpstr);
	strcpy(tmpstr, EXUSR);
	sprintf(EXUSR, "%s%s", test_path, tmpstr);

	strcpy(pool, nvram_safe_get("apps_pool"));
	strcpy(share, nvram_safe_get("apps_share"));
	strcpy(origver, nvram_safe_get("apps_ver"));
	sprintf(target_dir, "/shares/%s/%s", pool, share);
	sprintf(pool_dir, "/shares/%s", pool);
	sprintf(EXETC, "%s/.etc", pool_dir);

	system("rm -rf /shares/lib");
	system("rm -rf /shares/bin");
	system("rm -rf /shares/usr");
	system("rm -rf /shares/etc");
	doSystem("rm -rf %s", EXETC);

	mkdir_if_none(pool_dir);
#if 0
	if (!rename_if_dir_exist(pool_dir, share))
		mkdir_if_none(target_dir);
#else
	if (p_share = if_dircase_exist(pool_dir, share))
		strcpy(target_dir, p_share);
	else
		mkdir_if_none(target_dir);
#endif

	doSystem("ln -s %s /shares/lib", EXLIB);
	doSystem("ln -s %s /shares/bin", EXBIN);
	doSystem("ln -s %s /shares/usr", EXUSR);
	
	set_apps_ver();
	sprintf(NEWVER, nvram_safe_get("apps_ver"));

	if (!if_dircase_exist(target_dir, "Music"))
	{
		sprintf(tmpstr, "%s/Music", target_dir);
		if (mkdir_if_none(tmpstr)==0)
			dbg("Music DIR exist\n");
	}

	if (!if_dircase_exist(target_dir, "Video"))
	{
		sprintf(tmpstr, "%s/Video", target_dir);
		if (mkdir_if_none(tmpstr)==0)
			dbg("Video DIR exist\n");
	}

	if (!if_dircase_exist(target_dir, "Photo"))
	{
		sprintf(tmpstr, "%s/Photo", target_dir);
		if (mkdir_if_none(tmpstr)==0)
			dbg("PHOTO DIR exist\n");
	}

	if (!rename_if_dir_exist(target_dir, "Download"))
	{
		sprintf(tmpstr, "%s/Download", target_dir);
		if (mkdir_if_none(tmpstr)==0)
			dbg("DOWNLOAD DIR exist\n");
	}

	sprintf(tmpstr, "%s/Download/config", target_dir);
	mkdir_if_none(tmpstr);
	sprintf(tmpstr, "%s/Download/Complete", target_dir);
	mkdir_if_none(tmpstr);
	sprintf(tmpstr, "%s/Download/InComplete", target_dir);
	mkdir_if_none(tmpstr);
	sprintf(tmpstr, "%s/Download/.logs", target_dir);
	mkdir_if_none(tmpstr);	

	if (check_if_dir_exist("/tmp/harddisk/part0/.etc"))
		nvram_set("dmrd", "1");
	else
		nvram_set("dmrd", "0");

	doSystem("rm -rf %s", EXETC);
	mkdir_if_none(EXETC);
	doSystem("ln -s %s /shares/etc", EXETC);
	
	sprintf(tmpstr, "%s/asus_gift.conf", EXETC);
	if (check_if_file_exist(tmpstr)==1)
		dbg("FILE asus_gift exist\n");
	else
	{
		sprintf(tmpstr, "%s/gift-nasoc/bin/asus_gift.conf", EXUSR);
		sprintf(tmpstr2, "cp %s %s/. -f", tmpstr, EXETC);
		system(tmpstr2);
	}
	
	sprintf(tmpstr, "%s/.giFT", EXETC);
	if (mkdir_if_none(tmpstr)==0)
		dbg(".giFT DIR exist\n");
	else
	{
		sprintf(tmpstr2, "%s/.", tmpstr);
		sprintf(tmpstr4, "cp /shares/usr/.giFT/* %s -f", tmpstr2);
		system(tmpstr4);
		
		sprintf(tmpstr2, "%s/FastTrack", tmpstr);
		mkdir_if_none(tmpstr2);

		sprintf(tmpstr3, "%s/.", tmpstr2);
		sprintf(tmpstr4, "cp /shares/usr/.giFT/FastTrack/* %s -f", tmpstr3);
		system(tmpstr4);

		sprintf(tmpstr2, "%s/OpenFT", tmpstr);
		mkdir_if_none(tmpstr2);

		sprintf(tmpstr3, "%s/.", tmpstr2);
		sprintf(tmpstr4, "cp /shares/usr/.giFT/OpenFT/* %s -f", tmpstr3);
		system(tmpstr4);
		
		sprintf(tmpstr2, "%s/Gnutella", tmpstr);
		mkdir_if_none(tmpstr2);

		sprintf(tmpstr3, "%s/.", tmpstr2);
		sprintf(tmpstr4, "cp /shares/usr/.giFT/Gnutella/* %s -f", tmpstr3);
		system(tmpstr4);
		
		sprintf(tmpstr2, "%s/completed", tmpstr);
		mkdir_if_none(tmpstr2);
		sprintf(tmpstr2, "%s/completed/corrupted", tmpstr);
		mkdir_if_none(tmpstr2);
		sprintf(tmpstr2, "%s/ui", tmpstr);
		mkdir_if_none(tmpstr2);

		sprintf(tmpstr3, "%s/.", tmpstr2);
		sprintf(tmpstr4, "cp /shares/usr/.giFT/ui/* %s -f", tmpstr3);
		system(tmpstr4);
		
		sprintf(tmpstr, "%s/giFT", EXETC);
		mkdir_if_none(tmpstr);

		sprintf(tmpstr2, "%s/.", tmpstr);
		sprintf(tmpstr4, "cp /shares/usr/gift-nasoc/share/giFT/* %s -f", tmpstr2);
		system(tmpstr4);
		
		sprintf(tmpstr2, "%s/FastTrack", tmpstr);
		mkdir_if_none(tmpstr2);
		sprintf(tmpstr2, "%s/OpenFT", tmpstr);
		mkdir_if_none(tmpstr2);		
		sprintf(tmpstr2, "%s/Gnutella", tmpstr);
		mkdir_if_none(tmpstr2);
		sprintf(tmpstr2, "%s/completed", tmpstr);
		mkdir_if_none(tmpstr2);
		sprintf(tmpstr2, "%s/completed/corrupted", tmpstr);
		mkdir_if_none(tmpstr2);
		sprintf(tmpstr2, "%s/ui", tmpstr);
		mkdir_if_none(tmpstr2);

		sprintf(tmpstr3, "%s/.", tmpstr2);
		sprintf(tmpstr4, "cp /shares/usr/gift-nasoc/share/giFT/ui/* %s -f", tmpstr3);
		system(tmpstr4);
	}
	
	system("rm -rf /shares/DMSRoot");
	mkdir_if_none("/shares/DMSRoot");
	
	sprintf(tmpstr, "%s/Download/Complete", target_dir);
	doSystem("ln -s %s /shares/DMSRoot/Download", tmpstr);

	if (!if_dircase_exist(target_dir, "Video"))
	{
		sprintf(tmpstr, "%s/Video", target_dir);
		doSystem("ln -s %s /shares/DMSRoot/Video", tmpstr);
	}
	else
		doSystem("ln -s %s /shares/DMSRoot/Video", if_dircase_exist(target_dir, "Video"));

	if (!if_dircase_exist(target_dir, "Music"))
	{
		sprintf(tmpstr, "%s/Music", target_dir);
		doSystem("ln -s %s /shares/DMSRoot/Music", tmpstr);
	}
	else
		doSystem("ln -s %s /shares/DMSRoot/Music", if_dircase_exist(target_dir, "Music"));

	if (!if_dircase_exist(target_dir, "Photo"))
	{
		sprintf(tmpstr, "%s/Photo", target_dir);
		doSystem("ln -s %s /shares/DMSRoot/Photo", tmpstr);
	}
	else
		doSystem("ln -s %s /shares/DMSRoot/Photo", if_dircase_exist(target_dir, "Photo"));
	
	system("rm -rf /shares/dmathined");
	mkdir_if_none("/shares/dmathined");
	mkdir_if_none("/shares/dmathined/Download");

	sprintf(tmpstr, "%s/Download/config", target_dir);
	doSystem("ln -s %s /shares/dmathined/Download/config", tmpstr);
	sprintf(tmpstr, "%s/Download/Complete", target_dir);
	doSystem("ln -s %s /shares/dmathined/Download/Complete", tmpstr);
	sprintf(tmpstr, "%s/Download/InComplete", target_dir);
	doSystem("ln -s %s /shares/dmathined/Download/InComplete", tmpstr);
	sprintf(tmpstr, "%s/Download/.logs", target_dir);
	doSystem("ln -s %s /shares/dmathined/Download/.logs", tmpstr);

	system("chmod 777 /tmp/harddisk/part0/.apps/bin/*");
	system("chmod 777 /tmp/harddisk/part0/.apps/lib/*");
	system("chmod 777 /tmp/harddisk/part0/.apps/usr/gift-nasoc/lib/*");
	system("chmod 777 /tmp/harddisk/part0/.apps/usr/gift-nasoc/bin/*");
	system("chmod 777 /tmp/harddisk/part0/share");
	system("chmod 777 /tmp/harddisk/part0/share/Download");
	system("chmod 777 /tmp/harddisk/part0/share/Download/InComplete");
	system("chmod 777 /tmp/harddisk/part0/share/Download/Complete");

	eval("/sbin/test_of_var_files", "/tmp/harddisk/part0");	// J++
}

int is_apps_running()
{
	if (	pids("rtorrent") ||
		pids("dmathined") ||
		pids("giftd") ||
		pids("snarf")	)
	{
		return 1;
	}
	else
		return 0;
}

int is_dms_running()
{
	if (    pids("minidlna") ||
		pids("ushare")   )
                return 1;
	else
		return 0;
}

/*
 * st_ftp_mode: 0:no-ftp, 1:anonymous, 2:account 
 */

void stop_ftp() {
	int delay_count = 10;

	if (pids("vsftpd"))
		system("killall -SIGKILL vsftpd");
	else
		return;

	while (pids("vsftpd") && (delay_count-- > 0))
		sleep(1);

	unlink("/tmp/vsftpd.conf");

	logmessage("FTP Server", "daemon is stoped");
}

void stop_samba() {
	int delay_count = 10;

	if (!pids("smbd") && !pids("nmbd")) return;

	if (pids("smbd"))
		system("killall -SIGTERM smbd");
	if (pids("nmbd"))
		system("killall -SIGTERM nmbd");

        while ((pids("smbd") || pids("nmbd")) && (delay_count-- > 0))
                sleep(1);

	unlink("/etc/smb.conf");

	logmessage("Samba Server", "smb daemon is stoped");

	nvram_set("st_samba_mode_x", "0");      // 2007.11 James.
}

void restart_apps() {
	int is_apps_running_when_umount = is_apps_running();

	if (is_apps_running_when_umount)
	{
		if (pids("snarf"))
			system("killall -SIGKILL snarf");
		if (pids("rtorrent"))
			system("killall -SIGKILL rtorrent");
		if (pids("giftd"))
			system("killall -SIGKILL giftd");
		if (pids("dmathined"))
			system("killall -SIGKILL dmathined");

		nvram_set("dm_block", "0");
	}

	stop_samba();
	stop_ftp();
	stop_dms();
#ifdef CONFIG_USER_MTDAAPD
	stop_mt_daapd();
#endif

	if (is_apps_running_when_umount)
	{
		if (nvram_match("wan_route_x", "IP_Routed"))
		{
			dbg("rc_restart_firewall...\n");
			rc_restart_firewall();
		}
	}

	if (count_sddev_mountpoint())
	{
		pre_chk_mount();
		chk_partitions(USB_PLUG_ON);
		hotplug_usb_mass(NULL);
	}
}

void stop_ftpsamba() {
	stop_ftp();
	stop_samba();
}

void start_usb_apps() {
	dbg("*** start_usb_apps() ***\n");
	run_ftpsamba();
}
#if 0
void stop_usb_apps() {
	dbg("*** stop_usb_apps() ***\n");
	stop_apps();
	stop_ftpsamba();
	sleep(1);
}
#endif
void
run_ftp()
{
	start_ftp();
}

void
run_dms()
{
	start_dms();
}

#if 0
int start_dms() {
	char *DMS_ROOT = "/shares/DMSRoot";
	char name[32];
	char mymac[32];
	char path[256];
	char tmpstr2[256];
	int ret = 0;
	pid_t pid;
	char *argvs[5];

	if (!nvram_match("apps_dms", "1"))
		return;

	mkdir_if_none(DMS_ROOT);

	strcpy(name, nvram_safe_get("computer_name"));
	strcpy(mymac, nvram_safe_get("lan_hwaddr"));
	sprintf(tmpstr2, "/apps/bin/dms %s \"%s\" %s &", DMS_ROOT, name, mymac);

	ret = system(tmpstr2);
	if (!ret) {
		logmessage("Media Server", "daemon is started");

		return 0;
	}

	return 1;
}

#else

#if 0
void
write_dms_conf()	/* write /etc/ushare.conf */
{
	FILE *fp;

	if (check_if_file_exist("/etc/ushare.conf"))
		return;

	fp = fopen("/etc/ushare.conf", "w");

	if (fp == NULL)
		return;

	fprintf(fp, "USHARE_NAME=%s\n", nvram_safe_get("computer_name"));	// default is 'uShare'
	fprintf(fp, "USHARE_IFACE=%s\n", "br0");				// default is eth0
	fprintf(fp, "USHARE_PORT=%s\n", "");
	fprintf(fp, "USHARE_TELNET_PORT=%s\n", "");
//	fprintf(fp, "USHARE_DIR=%s,%s,%s\n",	"/shares/DMSRoot/Music",	// Directories to be shared
//						"/shares/DMSRoot/Photo",
//						"/shares/DMSRoot/Video");
//	fprintf(fp, "USHARE_DIR=%s\n",  "/shares/DMSRoot");
	fprintf(fp, "USHARE_DIR=%s\n",  "/media");
	fprintf(fp, "USHARE_OVERRIDE_ICONV_ERR=%s\n", "1");			// adds the non-iconv'ed string into the media list
	fprintf(fp, "USHARE_ENABLE_WEB=%s\n", "no");
	fprintf(fp, "USHARE_ENABLE_TELNET=%s\n", "no");
/*
	if (nvram_match("ushare_comp_mode", "1"))
	{
*/
		fprintf(fp, "USHARE_ENABLE_XBOX=%s\n", "yes");			// Use XboX 360 compatibility mode
		fprintf(fp, "USHARE_ENABLE_DLNA=%s\n", "no");			// Use DLNA profile
/*										// This is needed for PlayStation3 to work (among other devices)
	}
	else if (nvram_match("ushare_comp_mode", "2"))
	{
		fprintf(fp, "USHARE_ENABLE_XBOX=%s\n", "no");
		fprintf(fp, "USHARE_ENABLE_DLNA=%s\n", "yes");
	}
	else if (nvram_match("ushare_comp_mode", "3"))
	{
		fprintf(fp, "USHARE_ENABLE_XBOX=%s\n", "yes");
		fprintf(fp, "USHARE_ENABLE_DLNA=%s\n", "yes");
	}
	else
	{
		fprintf(fp, "USHARE_ENABLE_XBOX=%s\n", "no");
		fprintf(fp, "USHARE_ENABLE_DLNA=%s\n", "no");
	}
*/
	fprintf(fp, "\n");
	fclose(fp);
}
#endif

int
start_dms()
{
	if (!nvram_match("apps_dms", "1"))
		return 0;

	stop_dms();
#if 0
	if (nvram_match("dms_comp_mode", "1"))
	{
		write_dms_conf();
//		system("ushare -D");
		system("ushare 1>/dev/null 2>&1 &");
	}
	else
#endif
	{
		doSystem("minidlna -f /etc_ro/minidlna.conf -s %s -R", nvram_safe_get("br0hexaddr"));
	}

	nvram_set("apps_dms_ex", "1");

	if (/*(nvram_match("dms_comp_mode", "1") && pids("ushare")) || */pids("minidlna"))
	{
		logmessage("Media Server", "daemon is started");

		return 0;
	}

	return 1;
}

#ifdef CONFIG_USER_MTDAAPD
void
write_mt_daapd_conf()
{
	FILE *fp;

	if (check_if_file_exist("/etc/mt-daapd.conf"))
		return;

	fp = fopen("/etc/mt-daapd.conf", "w");

	if (fp == NULL)
		return;

	fprintf(fp, "web_root /etc_ro/web/admin-root\n");
	fprintf(fp, "port 3689\n");
	fprintf(fp, "admin_pw %s\n", nvram_safe_get("http_passwd"));
//	fprintf(fp, "db_dir /var/cache/mt-daapd\n");
	fprintf(fp, "db_dir /tmp/harddisk/part0/.itunes\n");
	fprintf(fp, "mp3_dir %s\n", "/media");
	if (!nvram_match("computer_name", "") && is_valid_hostname(nvram_safe_get("computer_name")))
		fprintf(fp, "servername %s\n", nvram_safe_get("computer_name"));
	else
		fprintf(fp, "servername %s\n", nvram_safe_get("productid"));
	fprintf(fp, "runas admin\n");
	fprintf(fp, "extensions .mp3,.m4a,.m4p,.aac,.ogg\n");
	fprintf(fp, "rescan_interval 300\n");
	fprintf(fp, "always_scan 1\n");
	fprintf(fp, "compress 1\n");

	fclose(fp);
}

int
start_mt_daapd()
{
	if (!nvram_match("apps_itunes", "1"))
		return;

	write_mt_daapd_conf();
	if (nvram_match("wan_route_x", "IP_Routed"))
		system("mt-daapd -m");
	else
		system("mt-daapd");

	nvram_set("apps_itunes_ex", "1");

	if (pids("mt-daapd"))
	{
		logmessage("iTunes Server", "daemon is started");

		return 0;
	}

	return 1;
}

void
stop_mt_daapd()
{
	int delay_count = 10;

	if (!pids("mt-daapd") && !pids("mDNSResponder"))
		return;

	if (pids("mDNSResponder"))
		system("killall mDNSResponder");

	if (pids("mt-daapd"))
		system("killall mt-daapd");

	while ((pids("mt-daapd") || pids("mDNSResponder")) && (delay_count-- > 0))
		sleep(1);

	logmessage("iTunes", "daemon is stoped");
}
#endif

void
stop_dms()
{
	int delay_count = 10;

	if (!is_dms_running())
		return;
	
	if (pids("minidlna"))
	{
		kill_pidfile_s("/var/run/minidlna.pid", SIGTERM);
	}

	if (pids("ushare"))
	{
		kill_pidfile_s("/var/run/ushare.pid", SIGTERM);
		usleep(100 * 1000);
		kill_pidfile_s("/var/run/ushare.pid", SIGKILL);
		remove("/var/run/ushare.pid");
	}

	while (is_dms_running() && (delay_count-- > 0))
		sleep(1);
	
	logmessage("Media Server", "daemon is stoped");
}
#endif

void
run_ftpsamba()
{
	run_ftp();
	sleep(1);
	run_samba();
}

void
run_samba()
{
	if (nvram_match("enable_samba", "0")) return;

	if (!nvram_match("st_samba_mode", "0"))	// 2007.11 James.
	{
		start_samba();
	}
}

#ifndef NO_DM
int
run_apps()
{
	int apps_comp=0;
	int apps_comp_pre=0;
	int apps_status=0;
	int apps_disk_free=0;
	int ret=0, i;
	char tmp[32];
	char tmpstr[64];
	char tmpstr2[64];
	FILE *fp;
	char *buf=NULL;
	int buflen=0;

	start_usbled();

	if (rename("/tmp/harddisk/part0/share/.apps", "/tmp/harddisk/part0/.apps") < 0)
	{
		dbg("mv apps fail\n");
		//return -1;
	}

	if ((apps_comp=ckeck_apps_completeness("/tmp/harddisk/part0"))==1)
		nvram_set("apps_comp", "1");
	else
	{
		dbg("invalid apps\n");   // tmp test
		nvram_set("apps_comp", "0");
		stop_usbled();
		return -1;
	}

	if ((!swap_check()) && (create_swap_file("/tmp/harddisk/part0/.swap") < 0))
	{
		stop_usbled();
		return -1;
	}

	init_apps();
	exec_apps();

	stop_usbled();
	return 0;
}
#else
int
run_apps()
{
	return 0;
}
#endif	// #ifndef NO_DM
#endif	// #ifdef DLM

int
check_if_dir_exist(const char *dirpath)
{
	struct stat stat_buf;

	if (!stat(dirpath, &stat_buf))
		return S_ISDIR(stat_buf.st_mode);
	else
		return 0;
}

int
check_if_file_exist(const char *filepath)
{
	struct stat stat_buf;

	if (!stat(filepath, &stat_buf))
		return S_ISREG(stat_buf.st_mode);
	else
		return 0;
}

int
mkdir_if_none(char *dir)
{
	DIR *dp;
	if (!(dp=opendir(dir)))
	{
		umask(0000);
		mkdir(dir, 0777);
		return 1;
	}
	closedir(dp);
	return 0;
}

int rename_if_dir_exist(const char *dir, const char *subdir)
{
	DIR *dirp;
	struct dirent *direntp;
	char oldpath[64], newpath[64];

	if (!dir || !subdir)
		return 0;

	if ((dirp = opendir(dir)))
	{
		while (dirp && (direntp = readdir(dirp)))
		{
			if (!strcasecmp(direntp->d_name, subdir) && strcmp(direntp->d_name, subdir))
			{
				sprintf(oldpath, "%s/%s", dir, direntp->d_name);
				sprintf(newpath, "%s/%s", dir, subdir);
				rename(oldpath, newpath);
				return 1;
			}
		}

		closedir(dirp);
	}

	return 0;
}

char *if_dircase_exist(const char *dir, const char *subdir)
{
	DIR *dirp;
	struct dirent *direntp;
	char oldpath[64];

	if (!dir || !subdir)
		return NULL;

	if ((dirp = opendir(dir)))
	{
		while (dirp && (direntp = readdir(dirp)))
		{
			if (!strcasecmp(direntp->d_name, subdir) && strcmp(direntp->d_name, subdir))
			{
				sprintf(oldpath, "%s/%s", dir, direntp->d_name);
				return strdup(oldpath);
			}
		}

		closedir(dirp);
	}

	return NULL;
}

unsigned long
file_size(const char *filepath)
{
	struct stat stat_buf;

	if (!stat(filepath, &stat_buf) && S_ISREG(stat_buf.st_mode))
		return ((unsigned long) stat_buf.st_size);
	else
		return 0;
}

//2008.10 magic{
int start_networkmap(void)
{
	char *networkmap_argv[] = {"networkmap", NULL};
	pid_t pid;

#if defined (W7_LOGO) || defined (WIFI_LOGO)
	return 0;
#endif

	if (!nvram_match("wan_route_x", "IP_Routed"))
		return 0;

	_eval(networkmap_argv, NULL, 0, &pid);
	
	return 0;
}

//2008.10 magic}

void stop_networkmap()
{
	int delay_count = 10;

	if (pids("networkmap"))
		system("killall networkmap");
	else
		return;

//	nvram_set("networkmap_fullscan", "0");

	while (pids("networkmap") && (delay_count-- > 0))
		sleep(1);
}

FILE* fopen_or_warn(const char *path, const char *mode)
{
	FILE *fp = fopen(path, mode);

	if (!fp)
	{
		dbg("hotplug USB: No such file or directory: %s\n", path);
		errno = 0;
		return NULL;
	}

	return fp;
}

void
umount_ejected()	// umount mount point(s) which was(were) already ejected
{
	FILE *procpt, *procpt2;
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96], line2[128], ptname[32];
	int dummy1, dummy2, ma, mi, sz;
	int active = 0;

	if (procpt = fopen_or_warn("/proc/mounts", "r"))
	while (fgets(line, sizeof(line), procpt))
	{
		if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
			continue;
		if (strncmp(devname, "/dev/sd", 7) == 0)
		{
			active = 0;
			if (procpt2 = fopen_or_warn("/proc/partitions", "r"))
			{
				while (fgets(line2, sizeof(line2), procpt2))
				{
					if (sscanf(line2, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
						continue;
					if (strcmp(devname+5, ptname) == 0)
					{
						active = 1;
						break;
					}
				}

				if (!active)
				{
					count_umount_ejected++;
					dbg("count_umount_ejected: %d\n", count_umount_ejected);

					dbg("umounting %s...\n", mpname);
					umount2(mpname, MS_NOEXEC | MS_NOSUID | 0x00000002);    // 0x00000002: MNT_DETACH
					rmdir(mpname);

					if (strlen(nvram_safe_get("usb_mnt_first_path")) && !strcmp(mpname, nvram_safe_get("usb_mnt_first_path")))
					{
						nvram_set("usb_mnt_first_path", "");
						if (atoi(nvram_safe_get("usb_mnt_first_path_port")) > 0)
						{
							nvram_set("usb_mnt_first_path_port", "0");

							nvram_set("apps_status_checked", "1");	// it means need to check
							nvram_set("apps_comp", "0");
							nvram_set("apps_disk_free", "0");
						}
						unlink("/tmp/harddisk/part0");
					}
				}

				fclose(procpt2);
			}
		}
	}

	if (procpt)
		fclose(procpt);
}

void
umount_dev(char *sd_dev)	// umount sd_dev
{
	FILE *procpt;
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96];
	int dummy1, dummy2;
	char swap_file_path[32], cmd[32];

	if (!sd_dev)
		return;

	if (procpt = fopen_or_warn("/proc/mounts", "r"))
	while (fgets(line, sizeof(line), procpt))
	{
		if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
			continue;

		if (!strncmp(devname, "/dev/sd", 7))
		{
			if (!strcmp(devname+5, sd_dev))
			{
				memset(swap_file_path, 0x0, sizeof(swap_file_path));
				sprintf(swap_file_path, "%s/.swap", mpname);
				
				if (check_if_file_exist(swap_file_path))
				{
					sprintf(cmd, "swapoff %s", swap_file_path);
					system(cmd);
					unlink(swap_file_path);
				}

				dbg("umounting %s ..\n", mpname);
				umount2(mpname, MS_NOEXEC | MS_NOSUID | 0x00000002);    // 0x00000002: MNT_DETACH
				rmdir(mpname);

				if (strlen(nvram_safe_get("usb_mnt_first_path")) && !strcmp(mpname, nvram_safe_get("usb_mnt_first_path")))
				{
					nvram_set("usb_mnt_first_path", "");
					if (atoi(nvram_safe_get("usb_mnt_first_path_port")) > 0)
					{
						nvram_set("usb_mnt_first_path_port", "0");

						nvram_set("apps_status_checked", "1");	// it means need to check
						nvram_set("apps_comp", "0");
						nvram_set("apps_disk_free", "0");
					}
					unlink("/tmp/harddisk/part0");
				}

				break;
			}
		}
	}

	if (procpt)
		fclose(procpt);
}

void
umount_dev_all(char *sd_dev)	// umount sd_dev
{
	FILE *procpt;
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96];
	int dummy1, dummy2;
	char swap_file_path[32], cmd[32];

	if (!sd_dev)
		return;

	if (procpt = fopen_or_warn("/proc/mounts", "r"))
	while (fgets(line, sizeof(line), procpt))
	{
		if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
			continue;

		if (!strncmp(devname, "/dev/sd", 7))
		{
			if (!strncmp(devname+5, sd_dev, 3))
			{
				memset(swap_file_path, 0x0, sizeof(swap_file_path));
				sprintf(swap_file_path, "%s/.swap", mpname);
				
				if (check_if_file_exist(swap_file_path))
				{
					sprintf(cmd, "swapoff %s", swap_file_path);
					system(cmd);
					unlink(swap_file_path);
				}

				dbg("umounting %s ..\n", mpname);
				umount2(mpname, MS_NOEXEC | MS_NOSUID | 0x00000002);    // 0x00000002: MNT_DETACH
				rmdir(mpname);

				if (strlen(nvram_safe_get("usb_mnt_first_path")) && !strcmp(mpname, nvram_safe_get("usb_mnt_first_path")))
				{
					nvram_set("usb_mnt_first_path", "");
					if (atoi(nvram_safe_get("usb_mnt_first_path_port")) > 0)
					{
						nvram_set("usb_mnt_first_path_port", "0");

						nvram_set("apps_status_checked", "1");	// it means need to check
						nvram_set("apps_comp", "0");
						nvram_set("apps_disk_free", "0");
					}
					unlink("/tmp/harddisk/part0");
				}
			}
		}
	}

	if (procpt)
		fclose(procpt);
}

void
umount_sddev_all()	// umount all sdxx
{
	FILE *procpt;
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96];
	int dummy1, dummy2;
	char swap_file_path[32], cmd[32];

	if (procpt = fopen_or_warn("/proc/mounts", "r"))
	while (fgets(line, sizeof(line), procpt))
	{
		if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
			continue;

		if (!strncmp(devname, "/dev/sd", 7))
		{
			dbg("umounting %s ..\n", mpname);
			umount2(mpname, MS_NOEXEC | MS_NOSUID | 0x00000002);    // 0x00000002: MNT_DETACH
			rmdir(mpname);

			if (strlen(nvram_safe_get("usb_mnt_first_path")) && !strcmp(mpname, nvram_safe_get("usb_mnt_first_path")))
			{
				nvram_set("usb_mnt_first_path", "");
				if (atoi(nvram_safe_get("usb_mnt_first_path_port")) > 0)
				{
					nvram_set("usb_mnt_first_path_port", "0");

					nvram_set("apps_status_checked", "1");	// it means need to check
					nvram_set("apps_comp", "0");
					nvram_set("apps_disk_free", "0");
				}
				unlink("/tmp/harddisk/part0");
			}
		}
	}

	if (procpt)
		fclose(procpt);
}

char *
find_sddev_by_mountpoint(char *mountpoint)
{
	FILE *procpt;
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96];
	int dummy1, dummy2;

	if (!mountpoint)
		return;

	if (procpt = fopen_or_warn("/proc/mounts", "r"))
	while (fgets(line, sizeof(line), procpt))
	{
		if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
			continue;
			
		if (!strncmp(devname, "/dev/sd", 7))
		{
			if (!strcmp(mpname, mountpoint))
			{
				fclose(procpt);
				return strdup(&devname[5]);
			}
		}
	}

	if (procpt)
		fclose(procpt);

	return NULL;
}

void
umount_usb_path(char *port)
{
	if (!port || (atoi(port) <= 0) || (atoi(port) > 2))
		return;

	char nvram_name[20], sdx1[5];
	int i, len;

//	if (!strcmp(nvram_name, "printer"))
//		return;

	dbg("umount usb path for port %d\n", atoi(port));
	
//	for (i = 0; i < index + 1 ; i++)
	for (i = 0; i < 16 ; i++)
	{
		sprintf(nvram_name, "usb_path%d_fs_path%d", atoi(port), i);
		if ((len = strlen(nvram_safe_get(nvram_name))))
		{
			dbg("umount %s (%s)\n", nvram_name, nvram_safe_get(nvram_name));
			umount_dev(nvram_safe_get(nvram_name));

			if (len == 3)
			{
				memset(sdx1, 0x0, 5);
				sprintf(sdx1, "%s1", nvram_safe_get(nvram_name));
				dbg("try to umount %s\n", sdx1);
				umount_dev(sdx1);
			}

			nvram_unset(nvram_name);
		}
	}

	sprintf(nvram_name, "usb_path%d_act", atoi(port));
	umount_dev_all(nvram_name);
#ifndef RTCONFIG_USB_MODEM
	nvram_set(nvram_name, "");
	nvram_set("usb_path", "");
#endif

	if (atoi(nvram_safe_get("usb_mnt_first_path_port")) == atoi(port))
	{
		nvram_set("usb_mnt_first_path", "");
		nvram_set("usb_mnt_first_path_port", "0");

		nvram_set("apps_status_checked", "1");	// it means need to check
		nvram_set("apps_comp", "0");
		nvram_set("apps_disk_free", "0");
	}
}

int
count_sddev_mountpoint()
{
	FILE *procpt;
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96];
	int dummy1, dummy2, count = 0;

	if (procpt = fopen("/proc/mounts", "r"))
	while (fgets(line, sizeof(line), procpt))
	{
		if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
			continue;
			
		if (strstr(devname, "/dev/sd"))
			count++;
	}

	if (procpt)
		fclose(procpt);

	return count;
}

int
found_mountpoint_with_apps()
{
	FILE *procpt;
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96], apps_path[32];
	int dummy1, dummy2;

	if (procpt = fopen("/proc/mounts", "r"))
	while (fgets(line, sizeof(line), procpt))
	{
		if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
			continue;
			
		if (strstr(devname, "/dev/sd"))
		{
			sprintf(apps_path, "%s/%s", mpname, ".apps");

			if (check_if_dir_exist(apps_path))
			{
				dbg("found partition with .apps: %s\n", mpname);

				nvram_set("usb_mnt_first_path", mpname);
				if (!strncmp(nvram_safe_get("usb_path1_act"), find_sddev_by_mountpoint(mpname), 3))
					nvram_set("usb_mnt_first_path_port", "1");
				else if (!strncmp(nvram_safe_get("usb_path2_act"), find_sddev_by_mountpoint(mpname), 3))
					nvram_set("usb_mnt_first_path_port", "2");
				else
					nvram_set("usb_mnt_first_path_port", "0");

				nvram_set("apps_status_checked", "1");	// it means need to check
				nvram_set("apps_comp", "0");
				nvram_set("apps_disk_free", "0");

				return 1;
			}
		}
	}

	if (procpt)
		fclose(procpt);

	return 0;
}

int
count_sddev_partition()
{
	FILE *procpt;
	char line[256], devname[32], ptname[32];
	int ma, mi, sz, count = 0;

	if (procpt = fopen_or_warn("/proc/partitions", "r"))
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
				continue;

			if (!strncmp(ptname, "sd", 2))
				count++;
		}

		fclose(procpt);
	}

	return count;
}

void
tmp_log(char *buf)
{
	FILE* fp;
	fp = fopen("/tmp/mdev02", "a");
	if (fp)
	{
		fprintf(fp, buf);
		fprintf(fp, "\n");
		fclose(fp);
	}
}

int
get_sdx_usbpath(char *sdx)
{
	char path[256], buffer[256];
	
	sprintf(path, "/sys/block/%s/device", sdx);
	if (readlink(path, buffer, sizeof(buffer)) != -1)
	{
		if (strstr(buffer, "1-1"))
		{
			return 1;
		}
		else if (strstr(buffer, "1-2"))
		{
			return 2;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

void usb_path_nvram(char *action, char *env_path)
{
	if (!action || !env_path)
		return;

	int i, usb_path = 0;
	char nvram_name[32];

	if (!strcmp(action, "add"))
	{
		usb_path = get_sdx_usbpath(&env_path[7]);
		if (usb_path == 0)
			usb_path = atoi(nvram_safe_get("usb_path"));

		if (env_path[10] == '\0')				// sda, sdb, sdc...
		{
			// clean up
			for (i = 0; i < 16 ; i++)
			{
				sprintf(nvram_name, "usb_path%d_fs_path%d", usb_path, i);
				nvram_unset(nvram_name);
			}

			sprintf(nvram_name, "usb_path%d_fs_path0", usb_path);
			nvram_set(nvram_name, &env_path[7]);
#ifndef RTCONFIG_USB_MODEM
			sprintf(nvram_name, "usb_path%d", usb_path);
			nvram_set(nvram_name, "storage");
#endif
			sprintf(nvram_name, "usb_path%d_act", usb_path);
			nvram_set(nvram_name, &env_path[7]);
//			sprintf(nvram_name, "usb_path%d_add", usb_path);
//			nvram_set(nvram_name, "1");
		}
		else if (env_path[14] == '1')				// sda1, sdb1, sdc1...
		{
			sprintf(nvram_name, "usb_path%d_fs_path0", usb_path);
			nvram_set(nvram_name, &env_path[11]);
		}
		else if (check_dev_sb_block_count(&env_path[11]))	// sda2, sdb2, sdc2...
		{
			sprintf(nvram_name, "usb_path%d_fs_path%c", usb_path, env_path[14] - 1);
			nvram_set(nvram_name, &env_path[11]);
		}
	}
/*
	else if (!strcmp(action, "remove"))
	{
		if (env_path[10] == '\0')	// sda, sdb, sdc...
		{
			sprintf(nvram_name, "usb_path%d_fs_path0", usb_path);
			nvram_set(nvram_name, "");
		}
		else if (env_path[14] == '1')	// sda1, sdb1, sdc1...
		{
			sprintf(nvram_name, "usb_path%d_fs_path0", usb_path);
			nvram_set(nvram_name, "");
		}
		else
		{
			sprintf(nvram_name, "usb_path%d_fs_path%c", usb_path, env_path[14] - 1);
			nvram_set(nvram_name, "");
		}
	}
*/
}

int
is_invalid_char_for_hostname(char c)
{
	int ret = 0;

	if (c < 0x20)
		ret = 1;
	else if (c >= 0x21 && c <= 0x2c)
		ret = 1;
	else if (c >= 0x2e && c <= 0x2f)
		ret = 1;
	else if (c >= 0x3a && c <= 0x40)
		ret = 1;
#if 0
	else if (c >= 0x5b && c <= 0x60)
		ret = 1;
#else
	else if (c >= 0x5b && c <= 0x5e)
		ret = 1;
	else if (c == 0x60)
		ret = 1;
#endif
	else if (c >= 0x7b)
		ret = 1;

//	printf("%c (0x%02x) is %svalid for hostname\n", c, c, (ret == 0) ? "  " : "in");

	return ret;
}

int
is_valid_hostname(const char *name)
{
	int ret = 1, len, i;

	len = strlen(name);
	if (len == 0)
	{
		ret = 0;
		goto ENDERR;
	}

	for (i = 0; i < len ; i++)
			if (is_invalid_char_for_hostname(name[i]))
		{
			ret = 0;
			break;
		}

ENDERR:
//	printf("%s is %svalid for hostname\n", name, (ret == 1) ? "  " : "in");
	return ret;
}

