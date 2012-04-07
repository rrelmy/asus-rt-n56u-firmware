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
#include <stdio.h>
#include <signal.h>
#include <sys/times.h>
#include <netconf.h>
#include <nvram/bcmnvram.h>
#include <rc.h>
#include <semaphore_mfp.h>
#include <ralink.h>
#include "iwlib.h"
#include <shutils.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>

#define APCLI_PERIOD 20

struct itimerval itv;
unsigned char channel_to_check[15];
char ht_extcha_to_check[15];
unsigned char signal_to_check[15];
unsigned char sta_ssid_ex[33];
int first_try_after_bootup = 1;
int first_link_down_then_up = 1;

extern int apcli_set(const char *pv_pair);
extern int ap_set(const char *pv_pair);

/*
struct timeval tv_ref, tv_now;
void print_reftime()
{
	gettimeofday(&tv_now, NULL);
	if ((tv_now.tv_usec - tv_ref.tv_usec) >= 0)
		dbg("sec: %ld, msec: %ld\n", tv_now.tv_sec-tv_ref.tv_sec, (tv_now.tv_usec-tv_ref.tv_usec)/1000);
	else
		dbg("sec: %ld, msec: %ld\n", tv_now.tv_sec-tv_ref.tv_sec-1, (1000000+tv_now.tv_usec-tv_ref.tv_usec)/1000);
}
*/

void
alarmtimer_apcli(unsigned long sec,unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

void
apcli_restart()
{
	char tmpstr[256];
	int flag_wep = 0;
//	int keylen = 0;
	int keyid = 0;

	if (!is_URE())
	{
		dbg("apcli_restart: Not in AP(-Client) mode!\n");
		return;
	}

	if (nvram_match("sta_ssid", ""))
	{
		apcli_set("ApCliEnable=0");
		return;
	}

	apcli_set("ApCliEnable=0");

	//SSID
	sprintf(tmpstr, "ApCliSsid=%s", nvram_safe_get("sta_ssid"));
	strcpy(sta_ssid_ex, nvram_safe_get("sta_ssid"));
	apcli_set(tmpstr);

	//AuthMode
	char *str = (char *) nvram_safe_get("sta_auth_mode");
	if (str)
	{
/*
		if (!strcmp(str, "open"))
		{
			apcli_set("ApCliAuthMode=OPEN");

			if (nvram_match("sta_wep_x", "0"))
				apcli_set("ApCliEncrypType=NONE");
			else
				apcli_set("ApCliEncrypType=WEP");
		}
		else if (!strcmp(str, "shared"))
		{
			apcli_set("ApCliAuthMode=SHARED");
			apcli_set("ApCliEncrypType=WEP");
		}
*/
		if (!strcmp(str, "open") && nvram_match("sta_wep_x", "0"))
		{
			apcli_set("ApCliAuthMode=OPEN");
			apcli_set("ApCliEncrypType=NONE");
		}
		else if (!strcmp(str, "open") || !strcmp(str, "shared"))
		{
			flag_wep = 1;
			apcli_set("ApCliAuthMode=WEPAUTO");
			apcli_set("ApCliEncrypType=WEP");
		}
		else if (!strcmp(str, "psk"))
		{
			if (nvram_match("sta_wpa_mode", "1"))
				apcli_set("ApCliAuthMode=WPAPSK");
			else if (nvram_match("sta_wpa_mode", "2"))
				apcli_set("ApCliAuthMode=WPA2PSK");

			//EncrypType
			if (nvram_match("sta_crypto", "tkip"))
				apcli_set("ApCliEncrypType=TKIP");
			else if (nvram_match("sta_crypto", "aes"))
				apcli_set("ApCliEncrypType=AES");

			//WPAPSK
			sprintf(tmpstr, "ApCliWPAPSK=%s", nvram_safe_get("sta_wpa_psk"));
			apcli_set(tmpstr);
		}
		else
		{
			apcli_set("ApCliAuthMode=OPEN");
			apcli_set("ApCliEncrypType=NONE");
		}
	}
	else
	{
		apcli_set("ApCliAuthMode=OPEN");
		apcli_set("ApCliEncrypType=NONE");
	}

	//EncrypType
//	if (!nvram_match("sta_wep_x", "0"))
	if (flag_wep)
	{
		//DefaultKeyID
		sprintf(tmpstr, "ApCliDefaultKeyID=%s", nvram_safe_get("sta_key"));
		apcli_set(tmpstr);

		//KeyType (0 -> Hex, 1->Ascii)
/*
		if ( (keylen = strlen(nvram_safe_get("sta_key1"))) == 5 || (keylen == 13))
			sprintf(tmpstr, "ApCliKey1Type=%s", "1");
		else
			sprintf(tmpstr, "ApCliKey1Type=%s", "0");
		apcli_set(tmpstr);
		if ( (keylen = strlen(nvram_safe_get("sta_key2"))) == 5 || (keylen == 13))
			sprintf(tmpstr, "ApCliKey2Type=%s", "1");
		else
			sprintf(tmpstr, "ApCliKey2Type=%s", "0");
		apcli_set(tmpstr);
		if ( (keylen = strlen(nvram_safe_get("sta_key3"))) == 5 || (keylen == 13))
			sprintf(tmpstr, "ApCliKey3Type=%s", "1");
		else
			sprintf(tmpstr, "ApCliKey3Type=%s", "0");
		apcli_set(tmpstr);
		if ( (keylen = strlen(nvram_safe_get("sta_key4"))) == 5 || (keylen == 13))
			sprintf(tmpstr, "ApCliKey4Type=%s", "1");
		else
			sprintf(tmpstr, "ApCliKey4Type=%s", "0");
		apcli_set(tmpstr);
*/

		//KeyStr
		if ( (keyid = atoi(nvram_safe_get("sta_key"))) == 1)
		{
			sprintf(tmpstr, "ApCliKey1=%s", nvram_safe_get("sta_key1"));
			apcli_set(tmpstr);
		}
		else if (keyid == 2)
		{
			sprintf(tmpstr, "ApCliKey2=%s", nvram_safe_get("sta_key2"));
			apcli_set(tmpstr);
		}
		else if (keyid == 3)
		{
			sprintf(tmpstr, "ApCliKey3=%s", nvram_safe_get("sta_key3"));
			apcli_set(tmpstr);
		}
		else if (keyid == 4)
		{
			sprintf(tmpstr, "ApCliKey4=%s", nvram_safe_get("sta_key4"));
			apcli_set(tmpstr);
		}
	}

	ifconfig(URE, IFUP, NULL, NULL);
	doSystem("brctl addif br0 %s", URE);

	apcli_set("ApCliEnable=1");
	nvram_set("sta_authorized", "0");
	dbg("Connecting to %s", nvram_safe_get("sta_ssid"));
}

int
get_apcli_status()
{
	if (!is_URE())
	{
		dbg("get_apcli_status: Not in AP(-Client) mode!\n");
		return 1;
	}

	int skfd;		/* generic raw socket desc.	*/
	int rc;
	struct wireless_info info;
	char buffer[128];

	/* Create a channel to the NET kernel. */
	if ((skfd = iw_sockets_open()) < 0)
	{
		perror("socket");
		return 0;
	}

	rc = get_info(skfd, URE, &info);

	/* Close the socket. */
  	close(skfd);

	if (	!rc &&
		info.b.has_essid && info.b.essid_on && info.has_ap_addr && strlen(info.b.essid) &&
		!strcmp(info.b.essid, nvram_safe_get("sta_ssid")))
	{
		if (nvram_match("sta_auth_mode", "psk"))
		{
			if (	nvram_match("sta_connected", "1") &&
				(nvram_match("sta_authorized", "1") || nvram_match("sta_authorized", "2")) )
			{
				dbg("Connected with: \"%s\" (%s)\n", 
					info.b.essid, iw_sawap_ntop(&info.ap_addr, buffer));
				return 2;
			}
			else
			{
				dbg("Connecting to \"%s\" (%s)\n", 
					info.b.essid, iw_sawap_ntop(&info.ap_addr, buffer));
				return 1;
			}
		}
		else
		{
			if (nvram_match("sta_connected", "1"))
			{
				dbg("Connected with: \"%s\" (%s)\n",
					info.b.essid, iw_sawap_ntop(&info.ap_addr, buffer));
      				return 2;
      			}
      			else
      			{
				dbg("Connecting to \"%s\" (%s)\n", 
					info.b.essid, iw_sawap_ntop(&info.ap_addr, buffer));
				return 1;
      			}
      		}
      	}
      	else
      	{
      		dbg("Disconnected...\n");
      		return 0;
      	}
}

void
set_apcli_ap()
{
	char tmpstr[256];
	int flag_wep = 0;
//	int keylen = 0;
	int keyid = 0;

	//SSID
	sprintf(tmpstr, "SSID=%s", nvram_safe_get("wl_ssid"));
	ap_set(tmpstr);

	//AuthMode
	char *str = (char *) nvram_safe_get("wl_auth_mode");
	if (str)
	{
		if (!strcmp(str, "open") && nvram_match("wl_wep_x", "0"))
		{
			ap_set("AuthMode=OPEN");
			ap_set("EncrypType=NONE");
		}
		else if (!strcmp(str, "open") || !strcmp(str, "shared"))
		{
			flag_wep = 1;
			ap_set("AuthMode=WEPAUTO");
			ap_set("EncrypType=WEP");
		}
		else if (!strcmp(str, "psk"))
		{
			if (nvram_match("wl_wpa_mode", "1"))
				ap_set("AuthMode=WPAPSK");
			else if (nvram_match("wl_wpa_mode", "2"))
				ap_set("AuthMode=WPA2PSK");
			else
				ap_set("AuthMode=WPAPSK");	// shound not happen...

			//EncrypType
			if (nvram_match("wl_crypto", "tkip"))
				ap_set("EncrypType=TKIP");
			else if (nvram_match("wl_crypto", "aes"))
				ap_set("EncrypType=AES");
			else
				ap_set("EncrypType=TKIP");	// shound not happen...

			//WPAPSK
			sprintf(tmpstr, "WPAPSK=%s", nvram_safe_get("wl_wpa_psk"));
			ap_set(tmpstr);
		}
		else
		{
			ap_set("AuthMode=OPEN");
			ap_set("EncrypType=NONE");
		}
	}
	else
	{
		ap_set("AuthMode=OPEN");
		ap_set("EncrypType=NONE");
	}

	//EncrypType
	if (flag_wep)
	{
		//DefaultKeyID
		sprintf(tmpstr, "DefaultKeyID=%s", nvram_safe_get("wl_key"));
		ap_set(tmpstr);

		//KeyType (0 -> Hex, 1->Ascii)
/*
		if ( (keylen = strlen(nvram_safe_get("wl_key1"))) == 5 || (keylen == 13))
			sprintf(tmpstr, "Key1Type=%d", 1);
		else
			sprintf(tmpstr, "Key1Type=%d", 0);
		ap_set(tmpstr);
		if ( (keylen = strlen(nvram_safe_get("wl_key2"))) == 5 || (keylen == 13))
			sprintf(tmpstr, "Key2Type=%d", 1);
		else
			sprintf(tmpstr, "Key2Type=%d", 0);
		ap_set(tmpstr);
		if ( (keylen = strlen(nvram_safe_get("wl_key3"))) == 5 || (keylen == 13))
			sprintf(tmpstr, "Key3Type=%d", 1);
		else
			sprintf(tmpstr, "Key3Type=%d", 0);
		ap_set(tmpstr);
		if ( (keylen = strlen(nvram_safe_get("wl_key4"))) == 5 || (keylen == 13))
			sprintf(tmpstr, "Key4Type=%d", 1);
		else
			sprintf(tmpstr, "Key4Type=%d", 0);
		ap_set(tmpstr);
*/

		//KeyStr
		if ( (keyid = atoi(nvram_safe_get("wl_key"))) == 1)
		{
			sprintf(tmpstr, "Key1=%s", nvram_safe_get("wl_key1"));
			ap_set(tmpstr);
		}
		else if (keyid == 2)
		{
			sprintf(tmpstr, "Key2=%s", nvram_safe_get("wl_key2"));
			ap_set(tmpstr);
		}
		else if (keyid == 3)
		{
			sprintf(tmpstr, "Key3=%s", nvram_safe_get("wl_key3"));
			ap_set(tmpstr);
		}
		else if (keyid == 4)
		{
			sprintf(tmpstr, "Key4=%s", nvram_safe_get("wl_key4"));
			ap_set(tmpstr);
		}
	}
}

int
site_survey_for_channel(const char *ssid, int *HT_EXT)
{
	if (!is_URE())
	{
		dbg("site_survey_for_channel: Not in AP(-Client) mode!\n");
		memset(channel_to_check, 0x0, 15);
		return -1;
	}

	if (!ssid || !strcmp(ssid, ""))
	{
		return -1;
	}

	int retval = 0, i = 0, apCount = 0, signal_max = -1, signal_tmp = -1, idx = -1;
	char data[8192];
	char ssid_str[256];
	char header[128];
	char tmp_wmode[8];
	char tmp_bsstype[4];
	struct iwreq wrq;
	SSA *ssap;
	int check_for_hidden_ap = atoi(nvram_safe_get("sta_check_ha"));
	int count_ap = 0;
	int channellistnum;
	int commonchannel;
	int centralchannel;
	int ht_extcha = 1;
	int ht_extcha_max;
	int wep = 0;

	if (!nvram_match("sta_wep_x", "0") || nvram_match("sta_auth_mode", "psk"))
		wep = 1;

	memset(data, 0x00, 255); 
	strcpy(data, "SiteSurvey=1"); 
	wrq.u.data.length = strlen(data)+1; 
	wrq.u.data.pointer = data; 
	wrq.u.data.flags = 0; 

	spinlock_lock(SPINLOCK_SiteSurvey);
	if (wl_ioctl(WIF, RTPRIV_IOCTL_SET, &wrq) < 0)
	{
		spinlock_unlock(0);

		dbg("Site Survey fails\n");
		return -1;
	}
	spinlock_unlock(SPINLOCK_SiteSurvey);

	dbg("Look for SSID: %s\n", ssid);
	dbg("Please wait");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".");
	sleep(1);
	dbg(".\n");

	memset(data, 0x0, 8192);
	strcpy(data, "");
	wrq.u.data.length = 8192;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;

	if (wl_ioctl(WIF, RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0)
	{
		dbg("errors in getting site survey result\n");
		return -1;
	}

	/*	Countries in the g band region 0 (channels 1~11):
	 *	CA CANADA
	 *	CO COLOMBIA
	 *	DO DOMINICAN REPUBLIC
	 *	GT GUATEMALA
	 * 	MX MEXICO
	 *	NO NORWAY
	 *	PA PANAMA
	 *	PR PUERTO RICO
	 *	TW TAIWAN
	 *	US UNITED STATES
	 *	UZ UZBEKISTAN
	 */

	if (check_for_hidden_ap)
	{
		memset(channel_to_check, 0x0, 15);
		memset(ht_extcha_to_check, 0x0, 15);
		memset(signal_to_check, 0x0, 15);

		char *value = nvram_safe_get("wl_country_code");

		if (	(strcasecmp(value, "CA") == 0) || (strcasecmp(value, "CO") == 0) ||
			(strcasecmp(value, "DO") == 0) || (strcasecmp(value, "GT") == 0) ||
			(strcasecmp(value, "MX") == 0) || (strcasecmp(value, "NO") == 0) ||
			(strcasecmp(value, "PA") == 0) || (strcasecmp(value, "PR") == 0) ||
			(strcasecmp(value, "TW") == 0) || (strcasecmp(value, "US") == 0) ||
			(strcasecmp(value, "UZ") == 0))
		{
			channel_to_check[0] = 11;
			channellistnum = 11;
		}
		else if (!value ||
			(strcmp(value, "") == 0) || (strcasecmp(value, "DB") == 0)/* ||
			(strcasecmp(value, "JP") == 0)*/)
		{
			channel_to_check[0] = 14;
			channellistnum = 14;
		}
		else
		{
			channel_to_check[0] = 13;
			channellistnum = 13;
		}
	}

	memset(header, 0x0, sizeof(header));
//	sprintf(header, "%-3s%-33s%-18s%-8s%-15s%-9s%-8s%-2s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode", "NT");
	sprintf(header, "%-3s%-33s%-18s%-9s%-16s%-9s%-8s%-2s%-3s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode", "NT", " CC");
//	dbg("\n%s", header);
	dbg("\n%-3s%-33s%-18s%-9s%-16s%-9s%-8s%-2s%-3s%-3s\n", "Ch", "SSID", "BSSID", "Enc", "Auth", "Siganl(%)", "W-Mode", "NT", " CC", " EC");

	if (wrq.u.data.length > 0)
	{
		ssap=(SSA *)(wrq.u.data.pointer+strlen(header)+1);
		int len = strlen(wrq.u.data.pointer+strlen(header))-1;
		char *sp, *op;
 		op = sp = wrq.u.data.pointer+strlen(header)+1;

		while (*sp && ((len - (sp-op)) >= 0))
		{
			ssap->SiteSurvey[i].channel[2] = '\0';
			ssap->SiteSurvey[i].ssid[32] = '\0';
			ssap->SiteSurvey[i].bssid[17] = '\0';
			ssap->SiteSurvey[i].encryption[8] = '\0';
			ssap->SiteSurvey[i].authmode[15] = '\0';
			ssap->SiteSurvey[i].signal[8] = '\0';
			ssap->SiteSurvey[i].wmode[7] = '\0';
			ssap->SiteSurvey[i].bsstype[2] = '\0';
			ssap->SiteSurvey[i].centralchannel[2] = '\0';

			sp+=strlen(header);
			apCount=++i;
		}

		if (apCount)
		{
			for (i=0;i<apCount;i++)
			{
				commonchannel = atoi(trim_r(ssap->SiteSurvey[i].channel));
				centralchannel = atoi(trim_r(ssap->SiteSurvey[i].centralchannel));
				if (strstr(ssap->SiteSurvey[i].bsstype, "n") && (commonchannel != centralchannel))
				{
					if (commonchannel <= 4)
						ht_extcha = 1;
					else if (commonchannel > 4 && commonchannel < 8)
					{
						if (centralchannel < commonchannel)
							ht_extcha = 0;
						else
							ht_extcha = 1;
					}
					else if (commonchannel >= 8)
					{
						if ((channellistnum - commonchannel) < 4)
							ht_extcha = 0;
						else
						{
							if (centralchannel < commonchannel)
								ht_extcha = 0;
							else
								ht_extcha = 1;
						}
					}

//					dbg("%-3s%-33s%-18s%-8s%-15s%-9s%-8s%-2s\n",
					dbg("%-3s%-33s%-18s%-9s%-16s%-9s%-8s%-2s %-2s %d\n",
						ssap->SiteSurvey[i].channel,
						(char*)ssap->SiteSurvey[i].ssid,
						ssap->SiteSurvey[i].bssid,
						ssap->SiteSurvey[i].encryption,
						ssap->SiteSurvey[i].authmode,
						ssap->SiteSurvey[i].signal,
						ssap->SiteSurvey[i].wmode,
						ssap->SiteSurvey[i].bsstype,
						ssap->SiteSurvey[i].centralchannel,
						ht_extcha
					);
				}
				else
				{
					ht_extcha = -1;
					dbg("%-3s%-33s%-18s%-9s%-16s%-9s%-8s%-2s %-2s\n",
						ssap->SiteSurvey[i].channel,
						(char*)ssap->SiteSurvey[i].ssid,
						ssap->SiteSurvey[i].bssid,
						ssap->SiteSurvey[i].encryption,
						ssap->SiteSurvey[i].authmode,
						ssap->SiteSurvey[i].signal,
						ssap->SiteSurvey[i].wmode,
						ssap->SiteSurvey[i].bsstype,
						ssap->SiteSurvey[i].centralchannel
					);
				}

				if (	(ssid && strcmp(ssid, trim_r((char*)ssap->SiteSurvey[i].ssid)) == 0) &&
						((strncmp(ssap->SiteSurvey[i].encryption, "NONE", 4) == 0 && wep == 0) ||
						 (strncmp(ssap->SiteSurvey[i].encryption, "NONE", 4) && wep == 1))
				)	
				{
					if (strncmp(ssap->SiteSurvey[i].bssid, nvram_safe_get("sta_bssid"), 17) == 0 )
					{
						*HT_EXT = ht_extcha;
						return atoi(trim_r(ssap->SiteSurvey[i].channel));
					}

					if ((signal_tmp=atoi(trim_r(ssap->SiteSurvey[i].signal))) > signal_max)
					{
						signal_max = signal_tmp;
						idx = i;
						ht_extcha_max = ht_extcha;
					}

					if (check_for_hidden_ap)
					{
						count_ap++;
						channel_to_check[atoi(trim_r(ssap->SiteSurvey[i].channel))] = 1;

						if (atoi(trim_r(ssap->SiteSurvey[i].signal)) >= signal_to_check[atoi(trim_r(ssap->SiteSurvey[i].channel))])
						{
							ht_extcha_to_check[atoi(trim_r(ssap->SiteSurvey[i].channel))] = ht_extcha;
							signal_to_check[atoi(trim_r(ssap->SiteSurvey[i].channel))] = atoi(trim_r(ssap->SiteSurvey[i].signal));
						}
					}
				}
				else if (	(strlen(trim_r((char*)ssap->SiteSurvey[i].ssid)) == 0 && check_for_hidden_ap) &&
						((strncmp(ssap->SiteSurvey[i].encryption, "NONE", 4) == 0 && wep == 0) ||
						 (strncmp(ssap->SiteSurvey[i].encryption, "NONE", 4) && wep == 1))
				)
				{
					count_ap++;
					channel_to_check[atoi(trim_r(ssap->SiteSurvey[i].channel))] = 1;

					if (strncmp(ssap->SiteSurvey[i].bssid, nvram_safe_get("sta_bssid"), 17) == 0 )
					{
						*HT_EXT = ht_extcha;
						return atoi(trim_r(ssap->SiteSurvey[i].channel));
					}

					if (atoi(trim_r(ssap->SiteSurvey[i].signal)) >= signal_to_check[atoi(trim_r(ssap->SiteSurvey[i].channel))])
					{
						ht_extcha_to_check[atoi(trim_r(ssap->SiteSurvey[i].channel))] = ht_extcha;
						signal_to_check[atoi(trim_r(ssap->SiteSurvey[i].channel))] = atoi(trim_r(ssap->SiteSurvey[i].signal));
					}
				}
			}
			dbg("\n");
		}

		if (count_ap)
		{
			dbg("Found (hidden) AP...\n");
			return 0;
		}
		else if (idx != -1)
		{
			*HT_EXT = ht_extcha_max;
			return atoi(trim_r(ssap->SiteSurvey[idx].channel));
		}
		else
			return -1;
	}

	return -1;
}

#if 0
int
match_subnet(const char *ip1, const char *sb1, const char *ip2, const char *sb2)
{
	struct in_addr in_addr_ip1, in_addr_sb1, in_addr_ip2, in_addr_sb2;
	struct in_addr in_addr_sb;

	if (sb1 && sb2 && ip1 && ip2)
	{
		dbg("ip1: %s\n", ip1);
		dbg("sb1: %s\n", sb1);
		dbg("ip2: %s\n", ip2);
		dbg("sb2: %s\n", sb2);
	}

	if (sb1 && sb2)
	{
		inet_aton(sb1, &in_addr_sb1);
		inet_aton(sb2, &in_addr_sb2);
	
		if (in_addr_sb1.s_addr < in_addr_sb2.s_addr)
			in_addr_sb.s_addr = in_addr_sb1.s_addr;
		else
			in_addr_sb.s_addr = in_addr_sb2.s_addr;
	}
	else
		return 0;

	if (ip1 && ip2)
	{
		inet_aton(ip1, &in_addr_ip1);
		inet_aton(ip2, &in_addr_ip2);

		if ((in_addr_ip1.s_addr & in_addr_sb1.s_addr) == (in_addr_ip2.s_addr & in_addr_sb2.s_addr))
			return 1;
		else
			return 0;
	}
	else
		return 0;
}
#endif

int
dhcpc_apply_delayed()
{
	FILE *fp;

	/* write pid */
	if ((fp=fopen("/var/run/dhcpc_apply_delayed.pid", "w"))!=NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	sleep(20);
	system("/tmp/landhcpc apply");

	return 0;
}

void
apcli_connect(int apply_new_profile)
{
	// apply_new_profile == 1, new profile from ui
	// apply_new_profile == 2, first try after bootup

	if (!is_URE())
	{
		dbg("apcli_connect: Not in AP(-Client) mode!\n");
		alarm(APCLI_PERIOD);
		return;
	}
	else if (nvram_match("sta_ssid", ""))
		return;

	int channel, i;
	char tmpstr[11];
	int count_site_survey = 0;
	int count_apcli_restart = 0;
	int time_wait;
	int HT_EXT;

	if (apply_new_profile == 2)
	{
		channel = site_survey_for_channel(nvram_safe_get("sta_ssid"), &HT_EXT);

		if (!channel)	// special mode for hidden AP
		{
			dbg("Activate workaround for hidden AP...\n");

			for (i = 1; i <= channel_to_check[0]; i++)
			{
				if (channel_to_check[i])
				{
					dbg("Change to channel: %d\n", i);

					if (ht_extcha_to_check[i] != -1)
					{
						dbg("HT_EXTCHA: %d\n", ht_extcha_to_check[i]);
//						sprintf(tmpstr, "HtBw=%d", 1);
//						ap_set(tmpstr);
						sprintf(tmpstr, "HtExtcha=%d", ht_extcha_to_check[i]);
						ap_set(tmpstr);
					}

					sprintf(tmpstr, "Channel=%d", i);
					apcli_set(tmpstr);

//					apcli_restart();
					count_apcli_restart++;
					sleep(1);
					dbg(".");
					sleep(1);
					dbg(".");
					sleep(1);
					dbg(".");
					sleep(1);
					dbg(".");
					sleep(1);
					dbg(".\n");

					if (get_apcli_status())
						break;
				}
			}
		}
		else if (channel != -1)
		{
			count_site_survey = 1;

			if (channel != get_channel())
			{				
				dbg("Change to channel: %d\n", channel);

				if (HT_EXT != -1)
				{
					dbg("HT_EXTCHA: %d\n", HT_EXT);
//					sprintf(tmpstr, "HtBw=%d", 1);
//					ap_set(tmpstr);
					sprintf(tmpstr, "HtExtcha=%d", HT_EXT);
					ap_set(tmpstr);
				}

				sprintf(tmpstr, "Channel=%d", channel);
				apcli_set(tmpstr);
			}
		}
		else
		{
			alarm(5);
			return;
		}
	}
	else if (apply_new_profile == 1 || !get_apcli_status())
	{
		if (strlen(sta_ssid_ex) && strcmp(sta_ssid_ex, nvram_safe_get("sta_ssid")))
			apcli_set("ApCliEnable=0");

		channel = site_survey_for_channel(nvram_safe_get("sta_ssid"), &HT_EXT);

		if (!channel)	// special mode for hidden AP
		{
			dbg("Activate workaround for hidden AP...\n");

			for (i = 1; i <= channel_to_check[0]; i++)
			{
				if (channel_to_check[i])
				{
					dbg("Change to channel: %d\n", i);

					if (ht_extcha_to_check[i] != -1)
					{
						dbg("HT_EXTCHA: %d\n", ht_extcha_to_check[i]);
//						sprintf(tmpstr, "HtBw=%d", 1);
//						ap_set(tmpstr);
						sprintf(tmpstr, "HtExtcha=%d", ht_extcha_to_check[i]);
						ap_set(tmpstr);
					}

					sprintf(tmpstr, "Channel=%d", i);
					apcli_set(tmpstr);

					apcli_restart();
					count_apcli_restart++;
					sleep(1);
					dbg(".");
					sleep(1);
					dbg(".");
					sleep(1);
					dbg(".");
					sleep(1);
					dbg(".");
					sleep(1);
					dbg(".\n");

					if (get_apcli_status())
						break;
				}
			}
		}
		else if (channel != -1)
		{
			count_site_survey = 1;

			if (channel != get_channel())
			{				
				dbg("Change to channel: %d\n", channel);

				if (HT_EXT != -1)
				{
					dbg("HT_EXTCHA: %d\n", HT_EXT);
//					sprintf(tmpstr, "HtBw=%d", 1);
//					ap_set(tmpstr);
					sprintf(tmpstr, "HtExtcha=%d", HT_EXT);
					ap_set(tmpstr);
				}

				sprintf(tmpstr, "Channel=%d", channel);
				apcli_set(tmpstr);
			}

			if (apply_new_profile)
			{
				apcli_restart();
				count_apcli_restart++;
				sleep(1);
				dbg(".");
				sleep(1);
				dbg(".");
				sleep(1);
				dbg(".");
				sleep(1);
				dbg(".");
				sleep(1);
				dbg(".\n");
			}
		}
		else
		{
			alarm(5);
			return;
		}
	}

	time_wait = APCLI_PERIOD - 4 * count_site_survey - 5 * count_apcli_restart;

	if (time_wait > 0)
		alarm(time_wait);
	else
		alarm(1);
}

void catch_sig_apcli(int sig)
{
	if (sig == SIGUSR1)
	{
		if (is_URE())
		{
			alarmtimer_apcli(0, 0);

			sleep(1);
			system("/tmp/landhcpc apply");

			alarm(APCLI_PERIOD);
		}
	}
	else if (sig == SIGUSR2)
	{
		dbg("receive SIGUSR2 from wireless driver!\n");

		if (nvram_match("sta_auth_mode", "psk"))
		{
			if ( !(	nvram_match("sta_connected", "1") &&
				(nvram_match("sta_authorized", "1") || nvram_match("sta_authorized", "2")) ) )
			{
				dbg("this SIGUSR2 should be ignored!\n");
				return;
			}
		}
		else
		{
			if (!nvram_match("sta_connected", "1"))
			{
				dbg("this SIGUSR2 should be ignored!\n");
				return;
			}
		}

		if (first_link_down_then_up)
		{
			printf("[rc] first link down then up\n");	// tmp test
			first_link_down_then_up = 0;
			if (!kill_pidfile_s("/var/run/udhcpd.pid", SIGUSR1))
				kill_pidfile_s("/var/run/udhcpd.pid", SIGTERM);
		}
		else
		{
			dbg("perform dhcp lease release\n");
			kill_pidfile_s("/var/run/udhcpc_lan.pid", SIGUSR2);	// perform lease release
			dbg("perform dhcp lease renew\n");
			kill_pidfile_s("/var/run/udhcpc_lan.pid", SIGUSR1);	// perform lease renew
		}
	}
	else if (sig == SIGTSTP)
	{
		dbg("catch_sig_apcli: Stops timer!\n");
		alarmtimer_apcli(0, 0);

		if (is_URE())
		{
			kill_pidfile_s("/var/run/dhcpc_apply_delayed.pid", SIGTERM);	// kill pending process
			unlink("/var/run/dhcpc_apply_delayed.pid");

			if (nvram_match("ui_triggered", "2"))	// triggered by WPS HW button (sta_wps_stop() in ralink.c)
			{
				nvram_set("ui_triggered", "0");
				nvram_set("apcli_workaround", "0");

				strcpy(sta_ssid_ex, nvram_safe_get("sta_ssid"));
				apcli_connect(2);
			}
			else
			{
				nvram_set("ui_triggered", "1");
				nvram_set("apcli_workaround", "0");

				set_apcli_ap();
				apcli_connect(1);
			}
		}
		else
		{
			dbg("catch_sig_apcli: Not in AP(-Client) mode!!\n");
			alarm(APCLI_PERIOD);
		}
	}
	else if (sig == SIGALRM)
	{
		if (is_URE())
		{

			if (first_try_after_bootup)
			{
				first_try_after_bootup--;
				apcli_connect(2);
			}
			else
				apcli_connect(0);
				
		}
		else
		{
			dbg("catch_sig_apcli: Not in AP(-Client) mode!!\n");
			alarm(APCLI_PERIOD);
		}
	}
}

int
apcli_monitor()
{
	FILE *fp;
	char tmpstr[32];

	/* write pid */
	if ((fp=fopen("/var/run/apcli_monitor.pid", "w"))!=NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	nvram_set("apcli_workaround", "0");
	nvram_unset("sta_check_ha");

	sprintf(tmpstr, "ApcliMonitorPid=%d", getpid());
	ap_set(tmpstr);
	memset(sta_ssid_ex, 0x0, 33);
	strcpy(sta_ssid_ex, nvram_safe_get("sta_ssid"));

	signal(SIGUSR1, catch_sig_apcli);
	signal(SIGUSR2, catch_sig_apcli);
	signal(SIGTSTP, catch_sig_apcli);
	signal(SIGALRM, catch_sig_apcli);

	alarm(1);

	while (1)
	{
		pause();
	}

	return 0;
}
