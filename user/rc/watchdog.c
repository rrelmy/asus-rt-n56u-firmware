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
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

 
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <shutils.h>
#include <rc.h>
#include <stdarg.h>
#include <rtxxxx.h>

typedef unsigned char bool;

#include <wlioctl.h>
#include <syslog.h>
#include <nvram/bcmnvram.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <sys/wait.h>
#include <nvram/bcmutils.h>

#include <sys/ioctl.h>

#include <wps.h>
#include <ralink.h>

#define BCM47XX_SOFTWARE_RESET  0x40		/* GPIO 6 */
#define RESET_WAIT		5		/* seconds */
#define RESET_WAIT_COUNT	RESET_WAIT * 10 /* 10 times a second */

#define TEST_PERIOD		100		/* second */
#define NORMAL_PERIOD		1		/* second */
#define URGENT_PERIOD		100 * 1000	/* microsecond */	
#define RUSHURGENT_PERIOD	50 * 1000	/* microsecond */

#ifdef BTN_SETUP
#define SETUP_WAIT		3		/* seconds */
#define SETUP_WAIT_COUNT	SETUP_WAIT * 10 /* 10 times a second */
#define SETUP_TIMEOUT		60 		/* seconds */
#define SETUP_TIMEOUT_COUNT	SETUP_TIMEOUT * 10 /* 60 times a second */
#endif // BTN_SETUP

int ddns_timer = 1;
static int media_timer = 0;
static int nm_timer = 0;
static int cpu_timer = 0;
static int mem_timer = -1;
static int httpd_timer = 0;
static int u2ec_timer = 0;
static int pppd_timer = 0;

struct itimerval itv;
int watchdog_period = 0;
int watchdog_count = 0;
static int btn_pressed = 0;
static int btn_count = 0;
long sync_interval = 1;	// every 10 seconds a unit
int sync_flag = 0;
long timestamp_g = 0;
int stacheck_interval = -1;
#ifdef BTN_SETUP
int btn_pressed_setup = 0;
int btn_pressed_flag = 0;
int btn_count_setup = 0;
int btn_count_timeout = 0;
int wsc_timeout = 0;
int btn_count_setup_second = 0;
#endif

#ifdef CDMA
int cdma_down = 0;
int cdma_connect = 0;
#endif

int reboot_count = 0;
static int count_to_stop_wps = 0;
extern int g_wsc_configured;
extern int g_isEnrollee;
static int WscStatus_old = -1;
static int WscStatus_old_2g = -1;

void 
sys_exit()
{
	printf("[watchdog] sys_exit");

	kill(1, SIGTERM);
}
#ifdef HTTPD_CHECK
#define DETECT_HTTPD_FILE "/tmp/httpd_check_result"

int httpd_error_count = 0;

int
httpd_check()
{
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	if (nvram_get("login_timestamp") && !nvram_match("login_timestamp", ""))
	{
		httpd_error_count = 0;
		return 1;
	}

	int ret = 0;
	FILE *fp = NULL;
	char line[80], cmd[128];

	remove(DETECT_HTTPD_FILE);
	snprintf(cmd, sizeof(cmd), "/usr/sbin/httpdcheck 4 127.0.0.1:80 > %s", DETECT_HTTPD_FILE);
//	dbg("cmd: %s\n", cmd);
	system(cmd);

	if ((fp = fopen(DETECT_HTTPD_FILE, "r")) != NULL)
	{
		if ( fgets(line, sizeof(line), fp) != NULL )
		{
			if (strstr(line, "alive"))
			{
//				dbg("httpd is alive!\n");
				ret = 1;
			}
		}

		fclose(fp);
	}
	else
	{
		dbg("fopen %s error!\n", DETECT_HTTPD_FILE);
	}

	if (!ret)
		httpd_error_count++;
	else
		httpd_error_count = 0;

//	dbg("httpd_error_count: %d\n", httpd_error_count);

	if (httpd_error_count > 3)
	{
		dbg("httpd is so dead!!!\n");
		httpd_error_count = 0;
		return 0;
	}
	else
		return 1;
#else
	return 1;
#endif
}

int check_count_down = 3;
time_t wget_timestamp;
char wget_timestampstr[32];

int
httpd_check_v2()
{
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	time_t now = uptime();

//	if (!nvram_match("wan_route_x", "IP_Routed"))
//		return 1;

	if (check_count_down)
	{
		check_count_down--;
		httpd_error_count = 0;
		return 1;
	}

        httpd_timer = (httpd_timer + 1) % 2;
        if (httpd_timer) return 1;

	if (nvram_match("v2_debug", "1"))
		dbg("uptime: %d\n", now);

	if (nvram_get("login_timestamp") && ((unsigned long)(now - strtoul(nvram_safe_get("login_timestamp"), NULL, 10)) < 60))
	{
		if (nvram_match("v2_debug", "1"))
			dbg("user login within 1 minutu: %d\n", (unsigned long)(now - strtoul(nvram_safe_get("login_timestamp"), NULL, 10)));

		httpd_error_count = 0;
		return 1;
	}

	int ret = 0;
	FILE *fp = NULL;
	char line[80], cmd[128], url[80];

	/* httpd will not count 127.0.0.1 */
	//sprintf(url, "http://%s/httpd_check.htm", get_lan_ipaddr());
	sprintf(url, "http://%s/httpd_check.htm", "127.0.0.1");
	remove(DETECT_HTTPD_FILE);

	wget_timestamp = uptime();
	memset(wget_timestampstr, 0, 32);
	sprintf(wget_timestampstr, "%lu", wget_timestamp);
	nvram_set("wget_timestamp", wget_timestampstr);
/*
	if (nvram_get("login_timestamp") && !nvram_match("login_timestamp", ""))
	{
		httpd_error_count = 0;
		return 1;
	}
	else
*/
		eval("wget", "-q", url, "-O", DETECT_HTTPD_FILE, "&");

	if ((fp = fopen(DETECT_HTTPD_FILE, "r")) != NULL)
	{
		if ( fgets(line, sizeof(line), fp) != NULL )
		{
			if (strstr(line, "ASUSTeK"))
			{
				if (nvram_match("v2_debug", "1"))
					dbg("httpd is alive!\n");

				ret = 1;
			}
		}

		fclose(fp);
	}
	else
	{
		if (nvram_match("v2_debug", "1"))
			dbg("fopen %s error!\n", DETECT_HTTPD_FILE);

		if (pids("wget"))
			system("killall wget");
	}

	nvram_unset("wget_timestamp");

	if (!ret)
	{
		if (nvram_match("v2_debug", "1"))
			dbg("httpd no response!\n");

		httpd_error_count++;
	}
	else
		httpd_error_count = 0;

	if (nvram_match("v2_debug", "1"))
		dbg("httpd_error_count: %d\n", httpd_error_count);

	if (httpd_error_count > 2)
	{
		dbg("httpd is so dead!!!\n");
		httpd_error_count = 0;
		return 0;
	}
	else
		return 1;
#else
	return 1;
#endif
}
#endif

void 
ra_gpio_init()
{
	ralink_gpio_init(LED_POWER, GPIO_DIR_OUT);
#ifndef SR3
	ralink_gpio_init(BTN_RESET, GPIO_DIR_OUT);	// clear SR2 low active state
	ralink_gpio_write_bit(BTN_RESET, 1);		// clear SR2 low acitve state
#else
	ralink_gpio_init(BTN_RESET, GPIO_DIR_IN);	// SR3
#endif
	ralink_gpio_init(BTN_WPS, GPIO_DIR_IN);
}

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

int no_need_to_start_wps(int wps_mode)
{
	if (wps_mode)	// PIN
	{
		if (nvram_match("wps_band", "0"))
		{
			if (	nvram_match("wl_auth_mode", "shared") ||
				nvram_match("wl_auth_mode", "wpa") ||
				nvram_match("wl_auth_mode", "wpa2") ||
				nvram_match("wl_auth_mode", "radius") ||
				nvram_match("wl_radio_x", "0") ||
				nvram_match("sw_mode_ex", "3")	)
				return 1;
		}
		else
		{
			if (	nvram_match("rt_auth_mode", "shared") ||
				nvram_match("rt_auth_mode", "wpa") ||
				nvram_match("rt_auth_mode", "wpa2") ||
				nvram_match("rt_auth_mode", "radius") ||
				nvram_match("rt_radio_x", "0") ||
				nvram_match("sw_mode_ex", "3")	)
				return 1;
		}
	}
	else
	{
#if 0
		if (	nvram_match("wl_auth_mode", "shared") ||
			nvram_match("wl_auth_mode", "wpa") ||
			nvram_match("wl_auth_mode", "wpa2") ||
			nvram_match("wl_auth_mode", "radius") ||
			nvram_match("wl_radio_x", "0") ||
			nvram_match("rt_auth_mode", "shared") ||
			nvram_match("rt_auth_mode", "wpa") ||
			nvram_match("rt_auth_mode", "wpa2") ||
			nvram_match("rt_auth_mode", "radius") ||
			nvram_match("rt_radio_x", "0") ||
			nvram_match("sw_mode_ex", "3")	)
			return 1;
#else
		if (    nvram_match("rt_auth_mode", "shared") ||
			nvram_match("rt_auth_mode", "wpa") ||
			nvram_match("rt_auth_mode", "wpa2") ||
			nvram_match("rt_auth_mode", "radius") ||
			nvram_match("rt_radio_x", "0") ||
			nvram_match("sw_mode_ex", "3")  )
			return 1;
#endif
	}

	return 0;
}

void btn_check(void)
{
#ifdef BTN_SETUP
	if (btn_pressed_setup == BTNSETUP_NONE)
	{
#endif

#ifdef SR3
	if (!ralink_gpio_read_bit(BTN_RESET))	// reset button is on low phase
#else
	if (0)
#endif
	{
	/*--------------- Add BTN_RST MFG test ------------------------*/
		if (!nvram_match("asus_mfg", "0"))
		{
			nvram_set("btn_rst", "1");
		}
		else
		{
			if (!btn_pressed)
			{
				btn_pressed = 1;
				btn_count = 0;
				alarmtimer(0, URGENT_PERIOD);
			}
			else
			{	/* Whenever it is pushed steady */
				if (++btn_count > RESET_WAIT_COUNT)
				{
					dbg("You can release RESET button now!\n");

					btn_pressed = 2;
				}
				if (btn_pressed == 2)
				{
				/* 0123456789 */
				/* 0011100111 */
					if ((btn_count % 10) < 1 || ((btn_count % 10) > 4 && (btn_count % 10) < 7))
						LED_CONTROL(LED_POWER, LED_OFF);
					else
						LED_CONTROL(LED_POWER, LED_ON);
				}
			}
		} // end BTN_RST MFG test
	}
	else
	{
		if (btn_pressed == 1)
		{
			btn_count = 0;
			btn_pressed = 0;
			LED_CONTROL(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);
		}
		else if (btn_pressed == 2)
		{
			LED_CONTROL(LED_POWER, LED_OFF);
			alarmtimer(0, 0);
			system("erase /dev/mtd1");
//			kill(1, SIGTERM);
			sys_exit();
		}
	}

#ifdef BTN_SETUP
	}
	if (btn_pressed != 0) return;

	if (btn_pressed_setup < BTNSETUP_START)
	{
		if (!ralink_gpio_read_bit(BTN_WPS) && !no_need_to_start_wps(0))
		{
			/* Add BTN_EZ MFG test */
			if (!nvram_match("asus_mfg", "0"))
			{
				nvram_set("btn_ez", "1");
			}
			else
			{
				if (btn_pressed_setup == BTNSETUP_NONE)
				{
					btn_pressed_setup = BTNSETUP_DETECT;
					btn_count_setup = 0;
					alarmtimer(0, RUSHURGENT_PERIOD);
				}
				else
				{	/* Whenever it is pushed steady */
					if (++btn_count_setup > SETUP_WAIT_COUNT)
					{
						if (!nvram_match("sw_mode_ex", "3"))
						{
							stop_wsc();
							stop_wsc_2g();
							nvram_set("wps_enable", "0");

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
							nvram_set("wps_triggered", "1");	// psp fix
#endif

							btn_pressed_setup = BTNSETUP_START;
							btn_count_setup = 0;
							btn_count_setup_second = 0;
#if defined (W7_LOGO) || defined (WIFI_LOGO)
							if (nvram_match("wps_band", "0"))
								start_wsc_pbc();
							else
								start_wsc_pbc_2g();
#else
#if 0
							start_wsc_pbc_both();
#else
							nvram_set("wps_band", "1");
							start_wsc_pbc_2g();
#endif
#endif
							WscStatus_old = -1;
							WscStatus_old_2g = -1;
							wsc_timeout = 120*20;
						}
					}
				}
			} // end BTN_EZ MFG test
		} 
		else if (btn_pressed_setup == BTNSETUP_DETECT)
		{
			btn_pressed_setup = BTNSETUP_NONE;
			btn_count_setup = 0;
			LED_CONTROL(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);
		}
	}
	else 
	{
		if (!ralink_gpio_read_bit(BTN_WPS) && !no_need_to_start_wps(0))
		{
			/* Whenever it is pushed steady, again... */
			if (++btn_count_setup_second > SETUP_WAIT_COUNT)
			{
				if (!nvram_match("sw_mode_ex", "3"))
				{
					stop_wsc();
					stop_wsc_2g();
					nvram_set("wps_enable", "0");

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
					nvram_set("wps_triggered", "1");	// psp fix
#endif
					dbg("pushed again...\n");
					btn_pressed_setup = BTNSETUP_START;
					btn_count_setup_second = 0;
#if defined (W7_LOGO) || defined (WIFI_LOGO)
					if (nvram_match("wps_band", "0"))
						start_wsc_pbc();
					else
						start_wsc_pbc_2g();
#else
#if 0
					start_wsc_pbc_both();
#else
					nvram_set("wps_band", "1");
					start_wsc_pbc_2g();
#endif
#endif
					WscStatus_old = -1;
					WscStatus_old_2g = -1;
					wsc_timeout = 120*20;
				}
#if 0
				else
				{
					dbg("pushed again... do nothing...\n");
/*
					btn_pressed_setup = BTNSETUP_START;
					btn_count_setup_second = 0;
					sta_wps_pbc();
					wsc_timeout = 120*20;
*/
				}
#endif
			}
		}

		struct _WSC_CONFIGURED_VALUE wsc_value;
		int int_stop_wps_led = 0;
		int WscStatus = -1;
		int WscStatus_2g = -1;

		if (nvram_match("wps_mode", "1"))	// PIN
		{
			if (nvram_match("wps_band", "0"))
				WscStatus = getWscStatus();
			else
				WscStatus = getWscStatus_2g();
		}
		else					// PBC
		{
			WscStatus = getWscStatus();
			WscStatus_2g = getWscStatus_2g();
		}
		
		if (WscStatus_old != WscStatus)
		{
			WscStatus_old = WscStatus;
			dbg("WscStatus: %d\n", WscStatus);
		}

		if (nvram_match("wps_mode", "2") && WscStatus_old_2g != WscStatus_2g)
		{
			WscStatus_old_2g = WscStatus_2g;
			dbg("WscStatus_2g: %d\n", WscStatus_2g);
		}

		if (WscStatus == 2 || WscStatus_2g == 2)// Wsc Process failed
		{
			if (g_isEnrollee)
				;			// go on monitoring
			else
			{
				int_stop_wps_led = 1;
				dbg("%s", "Error occured. Is the PIN correct?\n");
			}
		}

		// Driver 1.9 supports AP PBC Session Overlapping Detection.
		if (WscStatus == 0x109 /* PBC_SESSION_OVERLAP */ || WscStatus_2g == 0x109)
		{
			dbg("PBC_SESSION_OVERLAP\n");
			int_stop_wps_led = 1;
		}

		if (nvram_match("wps_mode", "1"))	// PIN
		{
			if (WscStatus == 34 /* Configured*/)
			{
/*
				dbg("getWscProfile()\n");
				getWscProfile(WIF, &wsc_value, sizeof(WSC_CONFIGURED_VALUE));
*/
				if (!g_wsc_configured)
					g_wsc_configured = 1;

				int_stop_wps_led = 1;
				g_isEnrollee = 0;
			}
		}
		else					// PBC
		{
			if (WscStatus == 34 /* Configured*/ || WscStatus_2g == 34)
			{
				if (!g_wsc_configured)
					g_wsc_configured = 1;

				int_stop_wps_led = 1;
				g_isEnrollee = 0;
			}
		}

		if (int_stop_wps_led || --wsc_timeout == 0)
		{
			if(!nvram_match("sw_mode_ex", "3"))	// not AP mode
			{
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
				nvram_set("wps_triggered", "1");// psp fix
#endif
				wsc_timeout = 0;

				btn_pressed_setup = BTNSETUP_NONE;
				btn_count_setup = 0;
				LED_CONTROL(LED_POWER, LED_ON);
				alarmtimer(NORMAL_PERIOD, 0);
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
//				if (nvram_match("wps_band", "0"))
					stop_wsc();		// psp fix
//				else
					stop_wsc_2g();		// psp fix
				nvram_set("wps_enable", "0");	// psp fix
#endif
			}
			return;
		}

		++btn_count_setup;
		btn_count_setup = (btn_count_setup % 20);

		/* 0123456789 */
		/* 1010101010 */
		if ((btn_count_setup % 2) == 0 && (btn_count_setup > 10))
			LED_CONTROL(LED_POWER, LED_ON);
		else
			LED_CONTROL(LED_POWER, LED_OFF);
	}
#endif
}

void refresh_ntpc(void)
{
	if (nvram_match("wan_route_x", "IP_Routed") && (!has_wan_ip() || !found_default_route()))
		return;

	setenv("TZ", nvram_safe_get("time_zone_x"), 1);
	
	if (pids("ntpclient"))
		system("killall ntpclient");

	if (!pids("ntp"))
	{
		stop_ntpc();
		start_ntpc();
	}
	else
		kill_pidfile_s("/var/run/ntp.pid", SIGTSTP);
}

void reset_svc_radio_time();

static int ntp_first_refresh = 0;

int ntp_timesync(void)
{
	time_t now;
	struct tm local;

	if (sync_interval != -1)
	{
		sync_interval--;

		if (nvram_match("ntp_ready", "1") && !ntp_first_refresh)
		{
			ntp_first_refresh = 1;

			if (!nvram_match("ddns_updated", "1"))
				start_ddns();

			sync_interval = 4320;
			logmessage("ntp client", "time is synchronized to %s", nvram_safe_get("ntp_server0"));

			reset_svc_radio_time();
		}
		else if (sync_interval == 0)
		{
			time(&now);
			localtime_r(&now, &local);

			/* More than 2000 */
			if (local.tm_year > 100)
			{	
				sync_interval = 4320;
				logmessage("ntp client", "time is synchronized to %s", nvram_safe_get("ntp_server0"));
				system("date");

				reset_svc_radio_time();
			}
			else
			{
//				sync_interval = 6;
				sync_interval = 3;
			}

			refresh_ntpc();	
		}
	}
}

enum 
{
	URLACTIVE = 0,
	URLACTIVE1,
	RADIOACTIVE,
	RADIO2ACTIVE,
	ACTIVEITEMS,
} ACTIVE;

int svcStatus[ACTIVEITEMS] = { -1, -1, -1, -1};
char svcDate[ACTIVEITEMS][10];
char *svcTime[ACTIVEITEMS][20];

#define DAYSTART (0)
#define DAYEND (60*60*23 + 60*59 + 59) // 86399

int timecheck_item(char *activeDate, char *activeTime)
{
	int current, active, activeTimeStart, activeTimeEnd;
	time_t now;
	struct tm *tm;

	setenv("TZ", nvram_safe_get("time_zone_x"), 1);

	time(&now);
	tm = localtime(&now);
	current = tm->tm_hour * 60 + tm->tm_min;
	active = 0;

	activeTimeStart = ((activeTime[0]-'0')*10 + (activeTime[1]-'0'))*60 + (activeTime[2]-'0')*10 + (activeTime[3]-'0');
	activeTimeEnd = ((activeTime[4]-'0')*10 + (activeTime[5]-'0'))*60 + (activeTime[6]-'0')*10 + (activeTime[7]-'0');

	if (activeDate[tm->tm_wday] == '1')
	{
		if (activeTimeEnd < activeTimeStart)
		{
			if ((current >= activeTimeStart && current <= DAYEND) ||
			   (current >= DAYSTART && current <= activeTimeEnd))
			{
				active = 1;
			}
			else
			{
				active = 0;
			}
		}
		else
		{
			if (current >= activeTimeStart && current <= activeTimeEnd)
			{
				active = 1;
			}
			else
			{
				active = 0;
			}
		}
	}

//	dbg("[watchdog] time check: %2d:%2d, active: %d\n", tm->tm_hour, tm->tm_min, active);

	return active;
}

void check_radio_time(const char *radio_time_name)
{
	char *radio_time = nvram_get(radio_time_name);
	char starttime[5], endtime[5];

	memset(starttime, 0, 5);
	memset(endtime, 0, 5);

	if ((!radio_time) || strlen(radio_time) != 8)
		goto err;

	strncpy(starttime, radio_time, 4);
	strncpy(endtime, radio_time + 4, 4);
//	dbG("%s starttime: %s\n", radio_time_name, starttime);
//	dbG("%s endtime: %s\n", radio_time_name, endtime);

	if (atoi(starttime) >= atoi(endtime))
		goto err;

	return;

err:
	dbG("fix %s as 00002359\n", radio_time_name);
	nvram_set(radio_time_name, "00002359");
}

extern int valid_url_filter_time();

/* Check for time-dependent service 	*/
/* 1. URL filter 			*/
/* 2. Wireless Radio			*/

int svc_timecheck(void)
{
	int activeNow;
/*
	if (valid_url_filter_time())
	{
		if (nvram_match("url_enable_x", "1"))
		{
			activeNow = timecheck_item(nvram_safe_get("url_date_x"), nvram_safe_get("url_time_x"));
			if (activeNow != svcStatus[URLACTIVE])
			{
				dbg("[watchdog] url filter 0: %s\n", activeNow ? "Enabled": "Disabled");
				svcStatus[URLACTIVE] = activeNow;
				stop_dns();
				start_dns();
			}
		}	

		if (nvram_match("url_enable_x_1", "1"))
		{
			activeNow = timecheck_item(nvram_safe_get("url_date_x"), nvram_safe_get("url_time_x_1"));

			if (activeNow != svcStatus[URLACTIVE1])
			{
				dbg("[watchdog] url filter 1: %s\n", activeNow ? "Enabled": "Disabled");
				svcStatus[URLACTIVE1] = activeNow;
				stop_dns();
				start_dns();
			}
		}
	}
*/
	if (!nvram_match("wl_radio_x", "0"))
	{
		/* Initialize */
		if (svcStatus[RADIOACTIVE] == -1 || nvram_match("reload_svc_wl", "1"))
		{
			if (nvram_match("reload_svc_wl", "1"))
			{
//				dbg("5G radio svc changed!\n");	
				nvram_set("reload_svc_wl", "0");
			}

			check_radio_time("wl_radio_time_x");
			strcpy(svcDate[RADIOACTIVE], nvram_safe_get("wl_radio_date_x"));
			strcpy(svcTime[RADIOACTIVE], nvram_safe_get("wl_radio_time_x"));
			svcStatus[RADIOACTIVE] = -2;

			if (!timecheck_item(svcDate[RADIOACTIVE], svcTime[RADIOACTIVE]))
				radio_main(0);
		}
		else
		{
			activeNow = timecheck_item(svcDate[RADIOACTIVE], svcTime[RADIOACTIVE]);
			if (activeNow != svcStatus[RADIOACTIVE])
			{
//				dbg("5G radio activity status changed!\n");

				svcStatus[RADIOACTIVE] = activeNow;

				if (activeNow)
					radio_main(1);
				else
					radio_main(0);
			}
//			else dbg("5G radio activity status: %d\n", activeNow);
		}
	}

	if (!nvram_match("rt_radio_x", "0"))
	{
		/* Initialize */
		if (svcStatus[RADIO2ACTIVE] == -1 || nvram_match("reload_svc_rt", "1"))
		{
			if (nvram_match("reload_svc_rt", "1"))
			{
//				dbg("2.4G radio svc changed!\n");
				nvram_set("reload_svc_rt", "0");
			}

			check_radio_time("rt_radio_time_x");
			strcpy(svcDate[RADIO2ACTIVE], nvram_safe_get("rt_radio_date_x"));
			strcpy(svcTime[RADIO2ACTIVE], nvram_safe_get("rt_radio_time_x"));
			svcStatus[RADIO2ACTIVE] = -2;

			if (!timecheck_item(svcDate[RADIO2ACTIVE], svcTime[RADIO2ACTIVE]))
				radio_main_rt(0);
		}
		else
		{
			activeNow = timecheck_item(svcDate[RADIO2ACTIVE], svcTime[RADIO2ACTIVE]);
			if (activeNow != svcStatus[RADIO2ACTIVE])
			{
//				dbg("2.4G radio activity status changed!\n");

				svcStatus[RADIO2ACTIVE] = activeNow;

				if (activeNow)
					radio_main_rt(1);
				else
					radio_main_rt(0);
			}
//			else dbg("2.4G radio activity status: %d\n", activeNow);
		}
	}

	return 0;
}

void reset_svc_radio_time()
{
	svcStatus[RADIOACTIVE] = -1;
	svcStatus[RADIO2ACTIVE] = -1;
}

/* Sometimes, httpd becomes inaccessible, try to re-run it */
httpd_processcheck(void)
{
	int httpd_is_missing = !pids("httpd");

	if (httpd_is_missing)
		printf("## httpd is gone! ##\n");

#ifndef ASUS_DDNS
	if (httpd_is_missing 
#ifdef HTTPD_CHECK
	    || !httpd_check_v2()
#endif
	)
#else	// 2007.03.27 Yau add for prevent httpd die when doing hostname check
	if (httpd_is_missing
#ifdef HTTPD_CHECK
	    || (/*!nvram_match("httpd_check_ddns", "1") && */!httpd_check_v2())
#endif
	)
#endif
	{
		printf("## restart httpd ##\n");

		stop_httpd();
		sleep(1);
#ifdef HTTPD_CHECK
		if (pids("httpdcheck"))
			system("killall -SIGTERM httpdcheck");
		remove(DETECT_HTTPD_FILE);
#endif
		start_httpd();
	}
}

void u2ec_processcheck(void)
{
        u2ec_timer = (u2ec_timer + 1) % 3;
        if (u2ec_timer) return 1;

	if (nvram_match("apps_u2ec_ex", "1") && (!pids("u2ec") || !pids("lpd")))
	{
		stop_u2ec();
		stop_lpd();
		start_u2ec();
		start_lpd();
	}
}

void media_processcheck(void)
{
	media_timer = (media_timer + 1) % 6;

	if (!media_timer)
	{
#if 0
		if (nvram_match("apps_dms_ex", "1") && nvram_match("dms_comp_mode", "1") && pids("ushare"))
		{
			if (nvram_match("ushare_debug", "1"))
			{
				system("date");
				dbg("ushare reloading media content...\n");
			}

			kill_pidfile_s("/var/run/ushare.pid", SIGHUP);
		}
#endif
		if (	nvram_match("wan_route_x", "IP_Routed") &&
			nvram_match("apps_itunes_ex", "1") &&
			pids("mt-daapd") &&
			!pids("mDNSResponder")	)
		{
			if (pids("mDNSResponder"))
				system("killall mDNSResponder");

			if (!nvram_match("computer_name", "") && is_valid_hostname(nvram_safe_get("computer_name")))
				doSystem("mDNSResponder %s thehost %s _daap._tcp. 3689 &", nvram_safe_get("lan_ipaddr_t"), nvram_safe_get("computer_name"));
			else
				doSystem("mDNSResponder %s thehost %s _daap._tcp. 3689 &", nvram_safe_get("lan_ipaddr_t"), nvram_safe_get("productid"));
		}
	}
}

int samba_error_count = 0;
#if 0
void samba_processcheck(void)
{
	if (!nvram_match("enable_samba", "1") || nvram_match("apps_smb_ex", "0"))
		return;

	if (pids("smbd") && !pids("nmbd"))
		samba_error_count++;

        if (samba_error_count > 3)
        {
//		dbg("nmbd is so dead!!!\n");
                samba_error_count = 0;

		eval("/sbin/nmbd", "-D", "-s", "/etc/smb.conf");
        }
}
#endif
void nm_processcheck(void)
{
	nm_timer = (nm_timer + 1) % 12;

	if (nm_timer) return;

	time_t now = uptime();
/*
	if (1)
	{
		dbg("fullscan_timestamp: %s\n", nvram_safe_get("fullscan_timestamp"));
		dbg("timeout: %d\n", (unsigned long)(now - strtoul(nvram_safe_get("fullscan_timestamp"), NULL, 10)));
	}
*/
	if (nvram_match("networkmap_fullscan", "1") &&
		((unsigned long)(now - strtoul(nvram_safe_get("fullscan_timestamp"), NULL, 10)) > 600) &&
			ddns_timer)
	{
//		dbg("networkmap is busy looping!\n");
		stop_networkmap();
	}
}

void pppd_processcheck()
{
	int unit = 0;
	char prefix[] = "wanXXXXXXXXXX_";
	char *wan_proto = nvram_safe_get("wan_proto");

	if (strcmp(wan_proto, "pppoe") &&
	    strcmp(wan_proto, "pptp") &&
	    strcmp(wan_proto, "l2tp"))
		return;

	pppd_timer = (pppd_timer + 1) % 15;

	if (pppd_timer) return;

	if (!pids("pppd"))
	{
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		start_pppd(prefix);

		if (pids("pppd"))
			logmessage("watchdog", "restart pppd");
	}
}
#if 0
unsigned int
get_cpu_usage()
{
	FILE *fp;
	unsigned int usage, ret = 0;
	char buf[256], cpu[32];
	int i;

	doSystem("/usr/bin/top -n 1 > %s", "/tmp/result_top");
        
	if (fp = fopen("/tmp/result_top", "r"))
	{
		while (fgets(buf, sizeof(buf), fp) != NULL)
		{
			i = sscanf(buf, "%32s %d ", cpu, &usage);
                        
			if (i && !strncmp(cpu, "CPU", 3))
			{
				ret = usage;
				break;
			}
		}
                
		fclose(fp);
		return ret;
	}

	return 0;
}
#endif
int high_cpu_usage_count = 0;

void
cpu_usage_minotor()
{
	cpu_timer = (cpu_timer + 1) % 6;
	if (cpu_timer) return;
#if 0
	unsigned int usage = get_cpu_usage();
#else
	unsigned int usage = cpu_main(0, NULL, 0);
#endif
	if (usage > 99)
		high_cpu_usage_count++;
	else
		high_cpu_usage_count = 0;

	if (high_cpu_usage_count > 30)
	{
		dbG("reboot for high CPU load !!!\n");
		dbG("reboot for high CPU load !!!\n");
		dbG("reboot for high CPU load !!!\n");
		sys_exit();	
	}
}

static void catch_sig(int sig)
{
	if (sig == SIGUSR1)
	{
		dbg("[watchdog] Catch SIGUSR1 for rc_service\n");
		if (nvram_get("rc_service"))
			service_handle();
	}
	else if (sig == SIGUSR2)
	{
//		dbg("[watchdog] Catch Reset to Default Signal 2\n");
	}
	else if (sig == SIGTSTP && !nvram_match("sw_mode_ex", "3"))
	{
		if (nvram_match("wps_oob_flag", "1"))
		{
#if defined (W7_LOGO) || defined (WIFI_LOGO)
			if (nvram_match("wps_band", "0"))
				if (nvram_match("wl_radio_x", "0"))
					return;
			else
				if (nvram_match("rt_radio_x", "0"))
					return;
#else
#if 0
			if (nvram_match("wl_radio_x", "0") || nvram_match("rt_radio_x", "0"))
#else
			if (nvram_match("wl_radio_x", "0") && nvram_match("rt_radio_x", "0"))
#endif
				return;
#endif
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
			nvram_set("wps_triggered", "1");	// psp fix
			count_to_stop_wps = 0;
#endif
			nvram_set("wps_oob_flag", "0");
			wsc_timeout = 0;
			btn_pressed_setup = BTNSETUP_NONE;
			btn_count_setup = 0;
			LED_CONTROL(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);

#if defined (W7_LOGO) || defined (WIFI_LOGO)
			if (nvram_match("wps_band", "0"))
				wps_oob();
			else
				wps_oob_2g();
#else
			wps_oob_both();
#endif
			WscStatus_old = -1;
			WscStatus_old_2g = -1;
		}
		else if (nvram_match("wps_start_flag", "3") || nvram_match("wps_start_flag", "4"))	// let the SW push button be the same as the HW push button
		{
#if defined (W7_LOGO) || defined (WIFI_LOGO)
			if (nvram_match("wps_band", "0"))
				if (nvram_match("wl_radio_x", "0"))
					return;
			else
				if (nvram_match("rt_radio_x", "0"))
					return;
#else
#if 0
			if (nvram_match("wl_radio_x", "0") || nvram_match("rt_radio_x", "0"))
#else
			if (nvram_match("wl_radio_x", "0") && nvram_match("rt_radio_x", "0"))
#endif
				return;
#endif
//			if (nvram_match("wl_radio_x", "1"))
			stop_wsc();
//			if (nvram_match("rt_radio_x", "1"))
			stop_wsc_2g();
			nvram_set("wps_enable", "0");
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
			nvram_set("wps_triggered", "1");	// psp fix
			count_to_stop_wps = 0;
			if (nvram_match("wps_start_flag", "3"))
                                nvram_set("wps_band", "1");
                        else
				nvram_set("wps_band", "0");
#endif
			nvram_set("wps_start_flag", "0");
			alarmtimer(NORMAL_PERIOD, 0);
			btn_pressed_setup = BTNSETUP_START;
			btn_count_setup = 0;
			btn_count_setup_second = 0;
#if defined (W7_LOGO) || defined (WIFI_LOGO)
			if (nvram_match("wps_band", "0"))
				start_wsc_pbc();
			else
				start_wsc_pbc_2g();
#else
#if 0
			start_wsc_pbc_both();
#else
			if (nvram_match("wps_band", "1"))
				start_wsc_pbc_2g();
			else
				start_wsc_pbc();
#endif
#endif
			WscStatus_old = -1;
			WscStatus_old_2g = -1;
			wsc_timeout = 120*20;
			alarmtimer(0, RUSHURGENT_PERIOD);
		}
		else if (nvram_match("wps_enable", "0"))
		{
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
			nvram_set("wps_triggered", "1");	// psp fix
			count_to_stop_wps = 0;
#endif
			wsc_timeout = 1;
			btn_pressed_setup = BTNSETUP_NONE;
			btn_count_setup = 0;
			LED_CONTROL(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);
//			if (nvram_match("wps_band", "0"))
//				if (nvram_match("wl_radio_x", "1"))
					stop_wsc();
//			else
//				if (nvram_match("rt_radio_x", "1"))
					stop_wsc_2g();
		}
		else if (nvram_match("wps_start_flag", "1"))
		{
			if (nvram_match("wps_band", "0"))
				if (nvram_match("wl_radio_x", "0"))
					return;
			else
				if (nvram_match("rt_radio_x", "0"))
					return;

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
			nvram_set("wps_triggered", "1");	// psp fix
			count_to_stop_wps = 15;
#endif
			nvram_set("wps_start_flag", "0");

			wsc_timeout = 1;
			btn_pressed_setup = BTNSETUP_NONE;
			btn_count_setup = 0;
			LED_CONTROL(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);

			WscStatus_old = -1;
			WscStatus_old_2g = -1;
			if (nvram_match("wps_band", "0"))
				start_wsc();
			else
				start_wsc_2g();
		}
		else if (nvram_match("wps_start_flag", "2"))
		{
			if (nvram_match("wps_mode", "1"))
			{
				if (nvram_match("wps_band", "0"))
					if (nvram_match("wl_radio_x", "0"))
						return;
				else
					if (nvram_match("rt_radio_x", "0"))
						return;
			}
			else
			{
#if defined (W7_LOGO) || defined (WIFI_LOGO)
				if (nvram_match("wps_band", "0"))
					if (nvram_match("wl_radio_x", "0"))
						return;
				else
					if (nvram_match("rt_radio_x", "0"))
						return;
#else
#if 0
				if (nvram_match("wl_radio_x", "0") || nvram_match("rt_radio_x", "0"))
#else
				if (nvram_match("wl_radio_x", "0") && nvram_match("rt_radio_x", "0"))
#endif
					return;
#endif
			}

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
			nvram_set("wps_triggered", "1");	// psp fix
			count_to_stop_wps = 0;
#endif
			nvram_set("wps_start_flag", "0");
			alarmtimer(NORMAL_PERIOD, 0);
			btn_pressed_setup = BTNSETUP_START;
			btn_count_setup = 0;

			if (nvram_match("wps_mode", "1"))
			{
				if (nvram_match("wps_pin_web", ""))
				{
					if (nvram_match("wps_band", "0"))
						wps_pin("0");
					else
						wps_pin_2g("0");
				}
				else
				{
					if (nvram_match("wps_band", "0"))
						wps_pin(nvram_safe_get("wps_pin_web"));
					else
						wps_pin_2g(nvram_safe_get("wps_pin_web"));
				}
			}
			else
			{
#if defined (W7_LOGO) || defined (WIFI_LOGO)
				if (nvram_match("wps_band", "0"))
					wps_pbc();
				else
					wps_pbc_2g();
#else
#if 0
				wps_pbc_both();
#else
				nvram_set("wps_band", "1");
				wps_pbc_2g();
#endif
#endif
			}

			WscStatus_old = -1;
			WscStatus_old_2g = -1;
			wsc_timeout = 120*20;
			alarmtimer(0, RUSHURGENT_PERIOD);
		}
	}
	else if (sig == SIGTTIN)
	{
		wsc_user_commit();
	}
}

int block_dm_count = 0;
void dm_block_chk()
{
	if(nvram_match("dm_block", "1"))
		++block_dm_count;
	else
		block_dm_count = 0;
	
	if(block_dm_count > 20)	// 200 seconds
	{	
		block_dm_count = 0;
		nvram_set("dm_block", "0");
		printf("[watchdog] unblock dm execution\n");	// tmp test
	}
}

extern int stop_service_type_99;
int chk_dns = 0;

/* wathchdog is runned in NORMAL_PERIOD, 1 seconds
 * check in each NORMAL_PERIOD
 *	1. button
 *
 * check in each NORAML_PERIOD*10
 *
 *      1. ntptime 
 *      2. time-dependent service
 *      3. http-process
 *      4. usb hotplug status
 */
void watchdog(void)
{
	/* handle button */
	btn_check();

	/* if timer is set to less than 1 sec, then bypass the following */
	if (itv.it_value.tv_sec == 0) return;

	if (nvram_match("asus_mfg", "1"))
	{
		system("rmmod hw_nat");
		if (pids("ntp"))
			system("killall -SIGTERM ntp");
		if (pids("ntpclient"))
			system("killall ntpclient");
		if (pids("udhcpc"))
			system("killall -SIGTERM udhcpc");
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
		if (pids("ots"))
			system("killall ots");
#endif
		stop_wanduck();
		stop_logger();
		stop_upnp();	// it may cause upnp cannot run
		stop_dhcpd();
		stop_dns();
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
		stop_pspfix();
#endif
		stop_wsc();
		stop_wsc_2g();
		stop_lltd();
		stop_networkmap();
		stop_httpd();
		stop_lpd();
		stop_u2ec();
		kill_pidfile_s("/var/run/linkstatus_monitor.pid", SIGTERM);
		if (pids("detectWan"))
			system("killall detectWan");
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
		stop_rstats();
		kill_pidfile_s("/var/run/detect_internet.pid", SIGTERM);
#endif
		if (pids("tcpcheck"))
			system("killall -SIGTERM tcpcheck");
#ifdef HTTPD_CHECK
		if (pids("httpdcheck"))
			system("killall -SIGTERM httpdcheck");
#endif
		if (pids("traceroute"))
			system("killall traceroute");
		if (pids("usbled"))
			system("killall -SIGTERM usbled");

		nvram_set("asus_mfg", "2");
	}

#if 0
	// reboot signal checking
	if (nvram_match("reboot", "1"))
	{
		printf("[watchdog] nvram match reboot\n");

		reboot_count++;
		if (reboot_count >= 2) 
		{
//			kill(1, SIGTERM);
			sys_exit();
		}

		return;
	}
#else
	if (nvram_match("reboot", "1")) return;
#endif

	if (stop_service_type_99) return;

	if (!nvram_match("asus_mfg", "0")) return;

	watchdog_period = (watchdog_period + 1) % 10;

	if (watchdog_period) return;

#ifdef BTN_SETUP
	if (btn_pressed_setup >= BTNSETUP_START) return;
#endif

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	if (count_to_stop_wps > 0)
	{
		count_to_stop_wps--;

		if (!count_to_stop_wps)
		{
//			if (nvram_match("wl_radio_x", "1"))
			stop_wsc();			// psp fix
//			if (nvram_match("rt_radio_x", "1"))
			stop_wsc_2g();			// psp fix
			nvram_set("wps_enable", "0");	// psp fix
		}
	}
#endif

	ddns_timer = (ddns_timer + 1) % 4320;

	if (nvram_match("asus_debug", "1"))
		mem_timer = (mem_timer + 1) % 60;

	if (!watchdog_count)
		watchdog_count++;
	else if (watchdog_count++ == 1)
	{
#if 0
		if (!strcmp(nvram_safe_get("rc_service"), "restart_upnp"))
			service_handle();
		else
#endif
		if (	nvram_match("router_disable", "0") &&
			nvram_match("upnp_enable", "1") &&
			nvram_match("upnp_started", "0")	)
		{
//			if (has_wan_ip())
			{
				dbg("[watchdog] starting upnp...\n");
				stop_upnp();
				start_upnp();
			}
		}
	}

	/* check for time-dependent services */
	svc_timecheck();

	/* http server check */
	httpd_processcheck();

	u2ec_processcheck();

	media_processcheck();
#if 0
	samba_processcheck();
#endif
	pppd_processcheck();

	if (nvram_match("wan_route_x", "IP_Routed"))
		nm_processcheck();

	cpu_usage_minotor();

	dm_block_chk();

	if (nvram_match("asus_debug", "1") && !mem_timer)
	{
		print_num_of_connections();
		dbg("Hardware NAT: %s\n", is_hwnat_loaded() ? "Enabled": "Disabled");
		dbg("Software QoS: %s\n", nvram_match("qos_enable", "1") ? "Enabled": "Disabled");
		dbg("pppd running: %s\n", pids("pppd") ? "Yes": "No");
#if 0
		dbg("CPU usage: %d%%\n", get_cpu_usage());
#else
		dbg("CPU usage: %d%%\n", cpu_main(0, NULL, 0));
#endif
		system("free");
		system("date");
		print_uptime();
	}

#ifdef CDMA
	/* CDMA_DOWN = 99, none
	 * CDMA_DOWN = 1, currently down
	 * CDMA_DOWN = 2, currently up
	 * CDMA_DOWN = 0, currently trying to connect
	 */
	if (nvram_match("cdma_down", "1"))
	{
		logmessage("CDMA client", "cdma is down(%d)!", cdma_down);

		cdma_down++;
		cdma_connec t = 0;

		if (cdma_down == 2)
		{
			printf("[watchdog] stop wan\n");
			stop_wan();
			start_wan();
		}
		else if (cdma_down >= 12) /* 2 minutes timeout for retry */
		{
			cdma_down = 0;
		}
	}
	else if (nvram_match("cdma_down", "0"))
	{
		logmessage("CDMA client", "cdma try connect(%d)!", cdma_connect);
		cdma_down = 0;
		cdma_connect++;

		if (cdma_connect > 12) /* 2 minitues timeout for connecting */
		{
			nvram_set("cdma_down", "1");
		}
	}
	else
	{
		cdma_down = 0;
		cdma_connect = 0;
	}
#endif

	if (nvram_match("wan_route_x", "IP_Routed"))
	{
		if (!is_phyconnected() || !has_wan_ip())
			return;

		/* sync time to ntp server if necessary */
		if (!nvram_match("wan_dns_t", "") && !nvram_match("wan_gateway_t", ""))
		{
			ntp_timesync();
		}

		if (	nvram_match("ddns_enable_x", "1") && 
			(!nvram_match("ddns_updated", "1") || !ddns_timer)
		)
		{
			start_ddns();
		}

		if (!ddns_timer)
		{
			stop_networkmap();
			start_networkmap();
		}
	}
	else
	{
		if (/*!nvram_match("lan_dns_t", "") && */!nvram_match("lan_gateway_t", ""))
			ntp_timesync();
	}
}

void
gpio_write(int gpionum, int value)
{
	ralink_gpio_init(gpionum, GPIO_DIR_OUT);
	ralink_gpio_write_bit(gpionum, value);
}

int 
watchdog_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;

#ifdef REMOVE
	/* Run it under background */
	switch (fork()) {
	case -1:
		exit(0);
		break;
	case 0:	
		// start in a new session
		(void) setsid();
		break;
	default:
		/* parent process should just die */
		_exit(0);
	}
#endif

	/* write pid */
	if ((fp = fopen("/var/run/watchdog.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	doSystem("iwpriv %s set WatchdogPid=%d", WIF, getpid());
	doSystem("iwpriv %s set WatchdogPid=%d", WIF2G, getpid());

	/* Start GPIO function */
	ra_gpio_init();

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGUSR1);
	sigaddset(&sigs_to_catch, SIGUSR2);
	sigaddset(&sigs_to_catch, SIGTSTP);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigaddset(&sigs_to_catch, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGUSR1, catch_sig);
	signal(SIGUSR2, catch_sig);
	signal(SIGTSTP, catch_sig);
	signal(SIGALRM, watchdog);
	signal(SIGTTIN, catch_sig);

	nvram_set("btn_rst", "0");
	nvram_set("btn_ez", "0");
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
//	if (!pids("ots"))
//		start_ots();
#endif
	setenv("TZ", nvram_safe_get("time_zone_x"), 1);

	/* set timer */
	alarmtimer(NORMAL_PERIOD, 0);

	if (	nvram_match("wan_route_x", "IP_Routed") &&
		(nvram_match("wan0_proto", "pppoe") || nvram_match("wan0_proto", "pptp") || nvram_match("wan0_proto", "l2tp"))
	)
	{
		if (nvram_match("wan0_proto", "pppoe") && nvram_match("wan0_pppoe_demand", "1") && !ppp0_as_default_route())
		{
//			system("route add -net default gateway 10.112.112.112 netmask 0.0.0.0 dev ppp0");
			preset_wan_routes("ppp0");
		}

		eval("ping", "8.8.8.8", "-c", "3");
	}

	/* Most of time it goes to sleep */
	while (1)
	{
		pause();
	}

	return 0;
}

int radio_main(int ctrl)
{
	if (!ctrl)
	{
		doSystem("iwpriv %s set RadioOn=0", WIF);
	}
	else
	{
		if (nvram_match("wl_radio_x", "1"))
			doSystem("iwpriv %s set RadioOn=1", WIF);
	}
}

int radio_main_rt(int ctrl)
{
	if (!ctrl)
	{
		doSystem("iwpriv %s set RadioOn=0", WIF2G);
	}
	else
	{
		if (nvram_match("rt_radio_x", "1"))
			doSystem("iwpriv %s set RadioOn=1", WIF2G);
	}
}
