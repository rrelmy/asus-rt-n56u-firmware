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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <nvram/bcmnvram.h>
#include <shutils.h>
#include <rc.h>
#include <netconf.h>
#include <nvparse.h>

#define foreach_x(x)	for (i=0; i<atoi(nvram_safe_get(x)); i++)
typedef unsigned char bool;	// 1204 ham

char *mac_conv(char *mac_name, int idx, char *buf);	// oleg patch
int mkdir_if_none(char *dir);				// oleg patch

extern int is_ap_mode();
void redirect_setting();

char *g_buf;
char g_buf_pool[1024];

// 0816 mv
bool
valid_autofw_port(const netconf_app_t *app)
{
	/* Check outbound protocol */
	if (app->match.ipproto != IPPROTO_TCP && app->match.ipproto != IPPROTO_UDP)
		return FALSE;

	/* Check outbound port range */
	if (ntohs(app->match.dst.ports[0]) > ntohs(app->match.dst.ports[1]))
		return FALSE;

	/* Check related protocol */
	if (app->proto != IPPROTO_TCP && app->proto != IPPROTO_UDP)
		return FALSE;

	/* Check related destination port range */
	if (ntohs(app->dport[0]) > ntohs(app->dport[1]))
		return FALSE;

	/* Check mapped destination port range */
	if (ntohs(app->to[0]) > ntohs(app->to[1]))
		return FALSE;

	/* Check port range size */
	if ((ntohs(app->dport[1]) - ntohs(app->dport[0])) !=
	    (ntohs(app->to[1]) - ntohs(app->to[0])))
		return FALSE;
	return TRUE;
}

void g_buf_init()
{
	g_buf = g_buf_pool;
}

char *g_buf_alloc(char *g_buf_now)
{
	g_buf += strlen(g_buf_now)+1;

	return (g_buf_now);
}

/*
void nvram_unsets(char *name, int count)
{
	char itemname_arr[32];
	int i;

	for (i=0; i<count; i++)
	{
		sprintf(itemname_arr, "%s%d", name, i);
		nvram_unset(itemname_arr);
	}
}
*/

char *proto_conv(char *proto_name, int idx)
{	
	char *proto;
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", proto_name, idx);
	proto=nvram_safe_get(itemname_arr);
		
	if (!strncasecmp(proto, "Both", 3)) strcpy(g_buf, "both");
	else if (!strncasecmp(proto, "TCP", 3)) strcpy(g_buf, "tcp");
	else if (!strncasecmp(proto, "UDP", 3)) strcpy(g_buf, "udp");
	else if (!strncasecmp(proto, "OTHER", 5)) strcpy(g_buf, "other");
	else strcpy(g_buf,"tcp");
	return (g_buf_alloc(g_buf));
}

char *protoflag_conv(char *proto_name, int idx, int isFlag)
{
	char *proto;
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", proto_name, idx);
	proto=nvram_safe_get(itemname_arr);

	strcpy(g_buf, "");
	
	if (!isFlag)
	{		
		if (strncasecmp(proto, "UDP", 3)==0) strcpy(g_buf, "udp");
		else strcpy(g_buf, "tcp");
	}
	else
	{	
		if (strlen(proto)>3)
		{
			strcpy(g_buf, proto+4);
		}			
		else strcpy(g_buf,"");
	}	
	return (g_buf_alloc(g_buf));
}
/*
char *portrange_ex_conv(char *port_name, int idx)
{
	char *port, *strptr;
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_safe_get(itemname_arr);

	strcpy(g_buf, "");
	
	if (!strncmp(port, ">", 1)) {
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
	}
	else if (!strncmp(port, "=", 1)) {
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
	}
	else if (!strncmp(port, "<", 1)) {
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
	}
	//else if (strptr=strchr(port, ':'))
	else if ((strptr=strchr(port, ':')) != NULL) //2008.11 magic oleg patch
	{		
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d-%d", atoi(itemname_arr), atoi(strptr+1));		
	}
	else if (*port)
	{
		sprintf(g_buf, "%d-%d", atoi(port), atoi(port));
	}
	else
	{
		//sprintf(g_buf, "");
		g_buf[0] = 0;	// oleg patch
	}
	
	return (g_buf_alloc(g_buf));
}
*/

char *portrange_ex2_conv(char *port_name, int idx, int *start, int *end)
{
	char *port, *strptr;
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_safe_get(itemname_arr);

	strcpy(g_buf, "");
	
	if (!strncmp(port, ">", 1)) 
	{
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
		*start=atoi(port+1);
		*end=65535;
	}
	else if (!strncmp(port, "=", 1)) 
	{
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
		*start=*end=atoi(port+1);
	}
	else if (!strncmp(port, "<", 1)) 
	{
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
		*start=1;
		*end=atoi(port+1);
	}
	//else if (strptr=strchr(port, ':'))
	else if ((strptr=strchr(port, ':')) != NULL) //2008.11 magic oleg patch
	{		
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d-%d", atoi(itemname_arr), atoi(strptr+1));	
		*start=atoi(itemname_arr);
		*end=atoi(strptr+1);
	}
	else if (*port)
	{
		sprintf(g_buf, "%d-%d", atoi(port), atoi(port));
		*start=atoi(port);
		*end=atoi(port);
	}
	else
	{
		//sprintf(g_buf, "");
		 g_buf[0] = 0;	// oleg patch
		*start=0;
		*end=0;
	}
	
	return (g_buf_alloc(g_buf));
}

char *portrange_ex2_conv_new(char *port_name, int idx, int *start, int *end)
{
	char *port, *strptr;
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_safe_get(itemname_arr);

	strcpy(g_buf, "");
	
	if (!strncmp(port, ">", 1)) 
	{
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
		*start=atoi(port+1);
		*end=65535;
	}
	else if (!strncmp(port, "=", 1)) 
	{
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
		*start=*end=atoi(port+1);
	}
	else if (!strncmp(port, "<", 1)) 
	{
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
		*start=1;
		*end=atoi(port+1);
	}
	else if ((strptr=strchr(port, ':')) != NULL)
	{		
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d:%d", atoi(itemname_arr), atoi(strptr+1));	
		*start=atoi(itemname_arr);
		*end=atoi(strptr+1);
	}
	else if (*port)
	{
		sprintf(g_buf, "%d:%d", atoi(port), atoi(port));
		*start=atoi(port);
		*end=atoi(port);
	}
	else
	{
		//sprintf(g_buf, "");
		 g_buf[0] = 0;	// oleg patch
		*start=0;
		*end=0;
	}
	
	return (g_buf_alloc(g_buf));
}

char *portrange_conv(char *port_name, int idx)
{
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", port_name, idx);
	strcpy(g_buf, nvram_safe_get(itemname_arr));	
	
	return (g_buf_alloc(g_buf));
}
/*
char *iprange_conv(char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32];
	char startip[16], endip[16];
	int i, j, k;
	
	sprintf(itemname_arr,"%s%d", ip_name, idx);
	ip=nvram_safe_get(itemname_arr);
	//strcpy(g_buf, "");
	 g_buf[0] = 0;	// 0313
	
	// scan all ip string
	i=j=k=0;
	
	while (*(ip+i))
	{
		if (*(ip+i)=='*') 
		{
			startip[j++] = '1';
			endip[k++] = '2';
			endip[k++] = '5';
			endip[k++] = '4';
			// 255 is for broadcast
		}
		else 
		{
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		i++;
	}	
	
	startip[j++] = 0;
	endip[k++] = 0;

	if (strcmp(startip, endip)==0)
		sprintf(g_buf, "%s", startip);
	else
		sprintf(g_buf, "%s-%s", startip, endip);
	return (g_buf_alloc(g_buf));
}

char *iprange_ex_conv(char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32];
	char startip[16], endip[16];
	int i, j, k;
	int mask;
	
	sprintf(itemname_arr,"%s%d", ip_name, idx);
	ip=nvram_safe_get(itemname_arr);
	strcpy(g_buf, "");
	
	// scan all ip string
	i=j=k=0;
	mask=32;
	
	while (*(ip+i))
	{
		if (*(ip+i)=='*') 
		{
			startip[j++] = '0';
			endip[k++] = '0';
			// 255 is for broadcast
			mask-=8;
		}
		else 
		{
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		i++;
	}	
	
	startip[j++] = 0;
	endip[k++] = 0;

	if (mask==32)
		sprintf(g_buf, "%s", startip);
	else if (mask==0)
		strcpy(g_buf, "");
	else sprintf(g_buf, "%s/%d", startip, mask);

	return (g_buf_alloc(g_buf));
}
*/
char *ip_conv(char *ip_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	sprintf(g_buf, "%s", nvram_safe_get(itemname_arr));
	return (g_buf_alloc(g_buf));
}

char *general_conv(char *ip_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	sprintf(g_buf, "%s", nvram_safe_get(itemname_arr));
	return (g_buf_alloc(g_buf));
}

char *filter_conv(char *proto, char *flag, char *srcip, char *srcport, char *dstip, char *dstport)
{
	char newstr[64];

	//printf("filter : %s,%s,%s,%s,%s,%s\n", proto, flag, srcip, srcport, dstip, dstport);
	
	strcpy(g_buf, "");		
										
	if (strcmp(proto, "")!=0)
	{
		sprintf(newstr, " -p %s", proto);
		strcat(g_buf, newstr);
	}				

	if (strcmp(flag, "")!=0)
	{
		//sprintf(newstr, " --tcp-flags %s RST", flag);
		sprintf(newstr, " --tcp-flags %s %s", flag, flag);
		strcat(g_buf, newstr);
	}			
		 
	if (strcmp(srcip, "")!=0)
	{
		if (strchr(srcip , '-'))
			sprintf(newstr, " --src-range %s", srcip);
		else	
			sprintf(newstr, " -s %s", srcip);
		strcat(g_buf, newstr);
	}				

	if (strcmp(srcport, "")!=0)
	{
		sprintf(newstr, " --sport %s", srcport);
		strcat(g_buf, newstr);
	}			

	if (strcmp(dstip, "")!=0)
	{
		if (strchr(dstip, '-'))
			sprintf(newstr, " --dst-range %s", dstip);
		else	
			sprintf(newstr, " -d %s", dstip);
		strcat(g_buf, newstr);
	}
			
	if (strcmp(dstport, "")!=0)
	{
		sprintf(newstr, " --dport %s", dstport);
		strcat(g_buf, newstr);
	}
	return (g_buf_alloc(g_buf));
	//printf("str: %s\n", g_buf);
}

void timematch_conv(char *mstr, char *nv_date, char *nv_time)
{
	char *datestr[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	char timestart[6], timestop[6];
	char *time, *date;
	int i, head;

	date = nvram_safe_get(nv_date);
	time = nvram_safe_get(nv_time);

	if (strlen(date)!=7||strlen(time)!=8) goto no_match;

	if (strncmp(date, "1111111", 7)==0 &&
	    strncmp(time, "00002359", 8)==0) goto no_match;
	
	i=0;
	strncpy(timestart, time, 2);
	i+=2;
	timestart[i++] = ':';
	strncpy(timestart+i, time+2, 2);
	i+=2;
	timestart[i]=0;
	i=0;
	strncpy(timestop, time+4, 2);
	i+=2;
	timestop[i++] = ':';
	strncpy(timestop+i, time+6, 2);
	i+=2;
	timestop[i]=0;

	sprintf(mstr, "-m time --timestart %s:00 --timestop %s:00 --days ",
//	sprintf(mstr, "-m time --timestart %s --timestop %s --days ",
			timestart, timestop);

	head=1;

	for (i=0;i<7;i++)
	{
		if (*(date+i)=='1')
		{
			if (head)
			{
				sprintf(mstr, "%s %s", mstr, datestr[i]);
				head=0;
			}
			else
			{	
				sprintf(mstr, "%s,%s", mstr, datestr[i]);
			}
		}
	}
	return;
	
no_match:
	//sprintf(mstr, "");
	mstr[0] = 0;	// oleg patch
	return;
}

char *iprange_ex_conv(char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32];
	char startip[16], endip[16];
	int i, j, k;
	int mask;

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	ip=nvram_safe_get(itemname_arr);
	strcpy(g_buf, "");

	// scan all ip string
	i=j=k=0;
	mask=32;

	while (*(ip+i))
	{
		if (*(ip+i)=='*')
		{
			startip[j++] = '0';
			endip[k++] = '0';
			// 255 is for broadcast
			mask-=8;
		}
		else
		{
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		++i;
	}

	startip[j++] = 0;
	endip[k++] = 0;

	if (mask==32)
		sprintf(g_buf, "%s", startip);
	else if (mask==0)
		strcpy(g_buf, "");
	else sprintf(g_buf, "%s/%d", startip, mask);

	return (g_buf_alloc(g_buf));
}

void
p(int step)
{
	dprintf("P: %d %s\n", step, g_buf);
}

void 
ip2class(char *lan_ip, char *netmask, char *buf)
{
	unsigned int val, ip;
	struct in_addr in;
	int i=0;

	// only handle class A,B,C	
	val = (unsigned int)inet_addr(netmask);
	ip = (unsigned int)inet_addr(lan_ip);
/*
	in.s_addr = ip & val;
	if (val==0xff00000) sprintf(buf, "%s/8", inet_ntoa(in));
	else if (val==0xffff0000) sprintf(buf, "%s/16", inet_ntoa(in));
	else sprintf(buf, "%s/24", inet_ntoa(in));
*/
	// oleg patch ~
	in.s_addr = ip & val;

	for (val = ntohl(val); val; i++) 
		val <<= 1;

	sprintf(buf, "%s/%d", inet_ntoa(in), i);
	// ~ oleg patch
	dprintf(buf);	
}

void convert_routes(void)
{
	/* Disable Static if it's not enable */	
	if (nvram_match("sr_enable_x", "0"))
	{
		nvram_set("lan_route", "");
		nvram_set("wan0_route", "");
		return;
	}

	int i;
	char *ip, *netmask, *gateway, *matric, *interface;
	//char wroutes[400], lroutes[400];
	char wroutes[1024], lroutes[1024], mroutes[1024];	// oleg patch

	wroutes[0] = 0;
	lroutes[0] = 0;	
	mroutes[0] = 0;	// oleg patch

	g_buf_init();
			
	//foreach_x("sr_num_x")
	if (nvram_match("sr_enable_x", "1")) foreach_x("sr_num_x")	// oleg patch
	{
		ip = general_conv("sr_ipaddr_x", i);
		netmask = general_conv("sr_netmask_x", i);
		gateway = general_conv("sr_gateway_x", i);
		matric = general_conv("sr_matric_x", i);
		interface = general_conv("sr_if_x", i);


		dprintf("%x %s %s %s %s %s\n", i, ip, netmask, gateway, matric, interface);

		if (!strcmp(interface, "WAN"))
		{		
			sprintf(wroutes, "%s %s:%s:%s:%d", wroutes, ip, netmask, gateway, atoi(matric)+1);
		}
		else if (!strcmp(interface, "MAN"))	// oleg patch
		{
			sprintf(mroutes, "%s %s:%s:%s:%d", mroutes, ip, netmask, gateway, atoi(matric)+1);
		} 
		else if (!strcmp(interface, "LAN"))
		{
			sprintf(lroutes, "%s %s:%s:%s:%d", lroutes, ip, netmask, gateway, atoi(matric)+1);
		}	
	}

	//Roly
	/* Disable Static if it's not enable */
/*	oleg patch
	if (nvram_match("sr_enable_x", "0"))
	{
		wroutes[0] = 0;
		lroutes[0] = 0;
	}
*/
	//printf("route: %s %s\n", lroutes, wroutes);
	nvram_set("lan_route", lroutes);
	nvram_set("wan0_route", wroutes);
	nvram_set("wan_route", mroutes);	// oleg patch
}

void write_static_leases(char *file)
{
	FILE *fp;
	char *ip, *mac;
	int i;

	fp=fopen(file, "w");

	if (fp==NULL) return;
	
	g_buf_init();
			
	foreach_x("dhcp_staticnum_x")
	{
		ip = general_conv("dhcp_staticip_x", i);
		mac = general_conv("dhcp_staticmac_x", i);
		fprintf(fp, "%s,%s\r\n", ip, mac);
	}
	fclose(fp);
}

#ifndef NOIPTABLES
void
//write_upnp_forward(FILE *fp, FILE *fp1, char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *lan_class, char *logaccept, char *logdrop)
write_upnp_forward(FILE *fp, char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *lan_class, char *logaccept, char *logdrop)	// oleg patch
{
	char name[] = "forward_portXXXXXXXXXX", value[512];
	char *wan_port0, *wan_port1, *lan_ipaddr, *lan_port0, *lan_port1, *proto;
	char *enable, *desc;
	int i;

	/* Set wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1,proto,enable,desc */
	for (i=0 ; i<15 ; i++)
	{
		snprintf(name, sizeof(name), "forward_port%d", i);

		strncpy(value, nvram_safe_get(name), sizeof(value));

		/* Check for LAN IP address specification */
		lan_ipaddr = value;
		wan_port0 = strsep(&lan_ipaddr, ">");
		if (!lan_ipaddr)
			continue;

		/* Check for LAN destination port specification */
		lan_port0 = lan_ipaddr;
		lan_ipaddr = strsep(&lan_port0, ":");
		if (!lan_port0)
			continue;

		/* Check for protocol specification */
		proto = lan_port0;
		lan_port0 = strsep(&proto, ":,");
		if (!proto)
			continue;

		/* Check for enable specification */
		enable = proto;
		proto = strsep(&enable, ":,");
		if (!enable)
			continue;

		/* Check for description specification (optional) */
		desc = enable;
		enable = strsep(&desc, ":,");

		/* Check for WAN destination port range (optional) */
		wan_port1 = wan_port0;
		wan_port0 = strsep(&wan_port1, "-");
		if (!wan_port1)
			wan_port1 = wan_port0;

		/* Check for LAN destination port range (optional) */
		lan_port1 = lan_port0;

		lan_port0 = strsep(&lan_port1, "-");
		if (!lan_port1)
			lan_port1 = lan_port0;

		/* skip if it's disabled */
		if ( strcmp(enable, "off") == 0 )
			continue;

		/* -A PREROUTING -p tcp -m tcp --dport 823 -j DNAT 
				 --to-destination 192.168.1.88:23  */
		if ( !strcmp(proto,"tcp") || !strcmp(proto,"both") )
		{
			//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %s "
			fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s "	// oleg patch
				  "-j DNAT --to-destination %s:%s\n"
					//, wan_ip, wan_port0, lan_ipaddr, lan_port0);	// oleg patch

			//fprintf(fp1, "-A FORWARD -p tcp "		// oleg patch
			//	 "-m tcp -d %s --dport %s -j %s\n"	// oleg patch
			//	 , lan_ipaddr, lan_port0, logaccept);	// oleg patch
			, wan_port0, lan_ipaddr, lan_port0);		// oleg patch
		}
		if ( !strcmp(proto,"udp") || !strcmp(proto,"both") ) {
			//fprintf(fp, "-A PREROUTING -p udp -m udp -d %s --dport %s "	// oleg patch
			fprintf(fp, "-A VSERVER -p udp -m udp --dport %s "		// oleg patch
				  "-j DNAT --to-destination %s:%s\n"
				  	//, wan_ip, wan_port0, lan_ipaddr, lan_port0);	// oleg patch

			//fprintf(fp1, "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n"	// oleg patch
			//	 , lan_ipaddr, lan_port0, logaccept);				// oleg patch
			, wan_port0, lan_ipaddr, lan_port0);					// oleg patch
		}
	}
}

/*
char *ipoffset(char *ip, int offset, char *tmp)
{
	unsigned int ip1, ip2, ip3, ip4;

	sscanf(ip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
	sprintf(tmp, "%d.%d.%d.%d", ip1, ip2, ip3, ip4+offset);

	dprintf("ip : %s\n", tmp);
	return (tmp);
}
*/

int apps_running_when_start_firewall = -1;

//nat_setting(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
void nat_setting(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *logaccept, char *logdrop)	// oleg patch
{
	//FILE *fp, *fp1;
	//char *proto, *flag, *srcip, *srcport, *dstip, *dstport;
	//char *setting, lan_class[32];
	FILE *fp;		// oleg patch
	char lan_class[32];	// oleg patch

	int i;
	int wan_port;
	//char *internal_ip, *internal_num;	
	//int num;
	char dstips[32], dstports[12];

	//if ((fp=fopen("/tmp/nat_rules", "w"))==NULL) return -1;
	//if ((fp1=fopen("/tmp/nat_forward_rules", "w"))==NULL) return -1;
	if ((fp=fopen("/tmp/nat_rules", "w"))==NULL) return;	// oleg patch

	fprintf(fp, "*nat\n"
		":PREROUTING ACCEPT [0:0]\n"
		":POSTROUTING ACCEPT [0:0]\n"
	  	/* ":OUTPUT ACCEPT [0:0]\n"); */
		":OUTPUT ACCEPT [0:0]\n"	// oleg patch
		":VSERVER - [0:0]\n");		// oleg patch

	//Log	
	//if (nvram_match("misc_natlog_x", "1"))
	// 	fprintf(fp, "-A PREROUTING -i %s -j LOG --log-prefix ALERT --log-level 4\n", wan_if);
// oleg patch ~
	/* VSERVER chain */
	if (inet_addr_(wan_ip))
		fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wan_ip);

	if (!nvram_match("wan0_ifname", wan_if) && inet_addr_(nvram_safe_get("wanx_ipaddr")))
		fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", nvram_safe_get("wanx_ipaddr"));
// ~ oleg patch
	if (nvram_match("misc_http_x", "1"))
	{
		wan_port=8080;
		if (!nvram_match("misc_httpport_x", ""))
			wan_port=atoi(nvram_safe_get("misc_httpport_x")); 	
		//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %d -j DNAT --to-destination %s:80\n", wan_ip, wan_port, lan_ip);
		fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %d -j DNAT --to-destination %s:%s\n",
			wan_port, lan_ip, nvram_safe_get("http_lanport"));	// oleg patch
	}
#if 0	
	if (nvram_match("apps_dl_share", "1"))
	{
		//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %s:%s -j DNAT --to %s\n", wan_ip, nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"), lan_ip);
		//fprintf(fp, "-A PREROUTING -p udp -m udp -d %s --dport %s:%s -j DNAT --to %s\n", wan_ip, nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"), lan_ip);
		fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s:%s -j DNAT --to %s\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"), lan_ip);	// oleg patch
		fprintf(fp, "-A VSERVER -p udp -m udp --dport %s:%s -j DNAT --to %s\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"), lan_ip);	// oleg patch
	}
#else
	if (apps_running_when_start_firewall)
	{
		fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s:%s -j DNAT --to %s\n", "10001", "10050", lan_ip);	// oleg patch
		fprintf(fp, "-A VSERVER -p udp -m udp --dport %s:%s -j DNAT --to %s\n", "10001", "10050", lan_ip);	// oleg patch
	}
#endif
	if (nvram_match("wan_nat_x", "1") && !nvram_match("upnp_enable", "0"))
	{
		// upnp port forward
		//write_upnp_forward(fp, fp1, wan_if, wan_ip, lan_if, lan_ip, lan_class, logaccept, logdrop);
		write_upnp_forward(fp, wan_if, wan_ip, lan_if, lan_ip, lan_class, logaccept, logdrop);	// oleg patch
	}

	// Port forwarding or Virtual Server
	if (nvram_match("wan_nat_x", "1") && nvram_match("vts_enable_x", "1"))
	{
		g_buf_init();

		foreach_x("vts_num_x")
		{
			char *proto;
			char *protono;
			char *port;
			char *lport;
			char *dstip;

			proto = proto_conv("vts_proto_x", i);
			protono = portrange_conv("vts_protono_x", i);
			port = portrange_conv("vts_port_x", i);
			lport = portrange_conv("vts_lport_x", i);
			dstip = ip_conv("vts_ipaddr_x", i);

			if (lport!=NULL && strlen(lport)!=0) 
			{
				sprintf(dstips, "%s:%s", dstip, lport);
				sprintf(dstports, "%s", lport);
			}
			else
			{
				sprintf(dstips, "%s:%s", dstip, port);
				sprintf(dstports, "%s", port);
			}

			if (strcmp(proto, "tcp")==0 || strcmp(proto, "both")==0)
			{
				if (lport!=NULL && strlen(lport)!=0) 
				{
					//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %s -j DNAT --to-destination %s\n", 
					//wan_ip, port, dstips);
					fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s -j DNAT --to-destination %s\n",
					port, dstips);	// oleg patch
				}
				else
				{
					//fprintf(fp, "-A PREROUTING -p tcp -m tcp -d %s --dport %s -j DNAT --to %s\n", 
					//wan_ip, port, dstip);
					fprintf(fp, "-A VSERVER -p tcp -m tcp --dport %s -j DNAT --to %s\n",
					port, dstip);	// oleg patch
				}

				//fprintf(fp1, "-A FORWARD -p tcp -m tcp -d %s --dport %s -j %s\n", 
				//	dstip, dstports, logaccept);
				//fprintf(fp, "-A FORWARD -p tcp -m tcp -d %s --dport %s -j %s\n",  dstip, dstports, logaccept);	// add back for conntrack patch
			}		
				
			if (strcmp(proto, "udp")==0 || strcmp(proto, "both")==0)
			{
				if (lport!=NULL && strlen(lport)!=0) 
				{
					//fprintf(fp, "-A PREROUTING -p udp -m udp -d %s --dport %s -j DNAT --to-destination %s\n", 
					//wan_ip, port, dstips);
					fprintf(fp, "-A VSERVER -p udp -m udp --dport %s -j DNAT --to-destination %s\n",
					port, dstips);	// oleg patch
				}
				else
				{
					//fprintf(fp, "-A PREROUTING -p udp -m udp -d %s --dport %s -j DNAT --to %s\n", 
					//wan_ip, port, dstip);
					fprintf(fp, "-A VSERVER -p udp -m udp --dport %s -j DNAT --to %s\n", port, dstip);	// oleg patch

				}
				//fprintf(fp1, "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n", 
				//	dstip, dstports, logaccept);	// oleg patch
				//fprintf(fp, "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n", dstip, dstports, logaccept);	// add back for conntrack patch
			}						
			if (strcmp(proto, "other")==0)
			{

				//fprintf(fp, "-A PREROUTING -p %s -d %s -j DNAT --to %s\n",
				//	protono, wan_ip, dstip);

				//fprintf(fp, "-A FORWARD -p %s -d %s -j %s\n", protono, dstip, logaccept);	// add back for conntrack patch

				fprintf(fp, "-A VSERVER -p %s -j DNAT --to %s\n",
					protono, dstip);	// oleg patch
			}							}	
	}	
//#if 0
	//if (nvram_match("wan_nat_x", "1") && !nvram_match("sp_battle_ips", "0"))
	if (nvram_match("wan_nat_x", "1") && !nvram_match("sp_battle_ips", "0") && inet_addr_(wan_ip))	// oleg patch
	{
		#define BASEPORT 6112
		#define BASEPORT_NEW 10000

		ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);

		/* run starcraft patch anyway */
		fprintf(fp, "-A PREROUTING -p udp -d %s --sport %d -j NETMAP --to %s\n", wan_ip, BASEPORT, lan_class);

		fprintf(fp, "-A POSTROUTING -p udp -s %s --dport %d -j NETMAP --to %s\n", lan_class, BASEPORT, wan_ip);

		//fprintf(fp, "-A FORWARD -p udp --dport %d -j %s\n",
		//			BASEPORT, logaccept);	// oleg patch
	}
//#endif
	// Exposed station	
	if (nvram_match("wan_nat_x", "1") && !nvram_match("dmz_ip", ""))
	{		
/*	oleg patch
		fprintf(fp, "-A PREROUTING -d %s -j DNAT --to %s\n", 
			wan_ip, nvram_safe_get("dmz_ip"));

 		fprintf(fp1, "-A FORWARD -d %s -j %s\n", 
			nvram_safe_get("dmz_ip"), logaccept);
*/
		fprintf(fp, "-A VSERVER -j DNAT --to %s\n", nvram_safe_get("dmz_ip"));	// oleg patch
	}

	if (nvram_match("wan_nat_x", "1"))
	{
		//fprintf(fp, "-A POSTROUTING -o %s -j MASQUERADE\n", wan_if);	
		// oleg patch ~
		if (inet_addr_(wan_ip))
			fprintf(fp, "-A POSTROUTING -o %s ! -s %s -j MASQUERADE\n", wan_if, wan_ip); 
		/* masquerade physical WAN port connection */
		if (!nvram_match("wan0_ifname", wan_if) && inet_addr_(nvram_safe_get("wanx_ipaddr")))
			fprintf(fp, "-A POSTROUTING -o %s ! -s %s -j MASQUERADE\n", 
				nvram_safe_get("wan0_ifname"), nvram_safe_get("wanx_ipaddr"));
		// ~ oleg patch

		// masquerade lan to lan
		ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);
		fprintf(fp, "-A POSTROUTING -o %s -s %s -d %s -j MASQUERADE\n", lan_if, lan_class, lan_class);

		porttrigger_setting_without_netconf(fp);
	}

	fprintf(fp, "COMMIT\n");
	
	fclose(fp);
	//fclose(fp1);	// oleg patch
	system("iptables-restore /tmp/nat_rules");
	// for rebuild the rule of wanduck gary add 2008.9
	//kill_pidfile_s("/var/run/wanduck.pid", SIGUSR1);	// 2008.03 James.

#ifdef WEB_REDIRECT
#ifndef RTCONFIG_USB_MODEM
	redirect_setting();
#endif
	nvram_set("wan_ready", "1");	// 2008.03 James.
#endif
}

#ifdef WEB_REDIRECT
void redirect_setting()
{
#ifndef RTCONFIG_USB_MODEM
	FILE *nat_fp = fopen("/tmp/nat_rules", "r");
#endif
	FILE *redirect_fp = fopen("/tmp/redirect_rules", "w+");
	FILE *fake_nat_fp = fopen("/tmp/fake_nat_rules", "w+");
	char tmp_buf[1024];
	char http_rule[256], dns_rule[256];
	//char *lan_ipaddr_t = nvram_safe_get("lan_ipaddr_t");
	//char *lan_netmask_t = nvram_safe_get("lan_netmask_t");
	char *lan_ipaddr_t = nvram_safe_get("lan_ipaddr");
	char *lan_netmask_t = nvram_safe_get("lan_netmask");

	if (redirect_fp == NULL) {
		dbg("*** Can't make the file of the redirect rules! ***\n");
		return;
	}
	if (fake_nat_fp == NULL) {
		dbg("*** create fake nat fules fail! ***\n");
		return;
	}

#ifndef RTCONFIG_USB_MODEM
	if (nat_fp != NULL) {
		memset(tmp_buf, 0, sizeof(tmp_buf));
		while ((fgets(tmp_buf, sizeof(tmp_buf), nat_fp)) != NULL
				&& strncmp(tmp_buf, "COMMIT", 6) != 0) {
			fprintf(redirect_fp, "%s", tmp_buf);
			memset(tmp_buf, 0, sizeof(tmp_buf));
		}

		fclose(nat_fp);
	}
	else{
#endif
		fprintf(redirect_fp, "*nat\n");
		fprintf(redirect_fp, ":PREROUTING ACCEPT [0:0]\n");
#ifndef RTCONFIG_USB_MODEM
	}
#endif
	fprintf(fake_nat_fp, "*nat\n");
	fprintf(fake_nat_fp, ":PREROUTING ACCEPT [0:0]\n");

	memset(http_rule, 0, sizeof(http_rule));
	memset(dns_rule, 0, sizeof(dns_rule));
	sprintf(http_rule, "-A PREROUTING -d ! %s/%s -p tcp --dport 80 -j DNAT --to-destination %s:18017\n", lan_ipaddr_t, lan_netmask_t, lan_ipaddr_t);
	sprintf(dns_rule, "-A PREROUTING -p udp --dport 53 -j DNAT --to-destination %s:18018\n", lan_ipaddr_t);

	fprintf(redirect_fp, "%s%s", http_rule, dns_rule);
	fprintf(redirect_fp, "COMMIT\n");
	fprintf(fake_nat_fp, "COMMIT\n");

	fclose(redirect_fp);
	fclose(fake_nat_fp);
}
#endif

/* Rules for LW Filter and MAC Filter
 * MAC ACCEPT
 *     ACCEPT -> MACS
 *             -> LW Disabled
 *                MACS ACCEPT
 *             -> LW Default Accept: 
 *                MACS DROP in rules
 *                MACS ACCEPT Default
 *             -> LW Default Drop: 
 *                MACS ACCEPT in rules
 *                MACS DROP Default
 *     DROP   -> FORWARD DROP 
 *
 * MAC DROP
 *     DROP -> FORWARD DROP
 *     ACCEPT -> FORWARD ACCEPT 
 */

int 	// 0928 add
default_filter_setting()
{
	FILE *fp;

	if ((fp=fopen("/tmp/filter.default", "w"))==NULL) return -1;

	fprintf(fp, "*filter\n:INPUT ACCEPT [0:0]\n:FORWARD ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]\n:logaccept - [0:0]\n:logdrop - [0:0]\n");
	fprintf(fp, "-A INPUT -m state --state INVALID -j DROP\n");
	fprintf(fp, "-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT\n");
	fprintf(fp, "-A INPUT -i lo -m state --state NEW -j ACCEPT\n");
	fprintf(fp, "-A INPUT -i br0 -m state --state NEW -j ACCEPT\n");
	fprintf(fp, "-A INPUT -j DROP\n");
	fprintf(fp, "-A FORWARD -m state --state INVALID -j DROP\n");
	fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT\n");
	fprintf(fp, "-A FORWARD -i br0 -o br0 -j ACCEPT\n");
	fprintf(fp, "-A FORWARD -i lo -o lo -j ACCEPT\n");
	fprintf(fp, "-A FORWARD -j DROP\n");
	fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");

	fprintf(fp,"-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP\" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");
	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	//system("iptables -F"); 
	system("iptables-restore /tmp/filter.default");
}

#ifdef WEBSTRFILTER
/* url filter corss midnight patch start */
int makeTimestr(char *tf)
{
	char *url_time = nvram_get("url_time_x");
	char *url_date = nvram_get("url_date_x");
	static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int i, comma = 0;

	memset(tf, 0, 256);

	if (!nvram_match("url_enable_x", "1"))
		return -1;

	if ((!url_date) || strlen(url_date) != 7 || !strcmp(url_date, "0000000") || !url_time)
	{
		printf("url filter get time fail\n");
		return -1;
	}

	sprintf(tf, "-m time --timestart %c%c:%c%c:00 --timestop %c%c:%c%c:59 --days ", url_time[0], url_time[1], url_time[2], url_time[3], url_time[4], url_time[5], url_time[6], url_time[7]);
//	sprintf(tf, " -m time --timestart %c%c:%c%c --timestop %c%c:%c%c --days ", url_time[0], url_time[1], url_time[2], url_time[3], url_time[4], url_time[5], url_time[6], url_time[7]);

	for (i=0; i<7; ++i)
	{
		if (url_date[i] == '1')
		{
			if (comma == 1)
				strncat(tf, ",", 1);

			strncat(tf, days[i], 3);
			comma = 1;
		}
	}

	printf("# url filter time module str is [%s]\n", tf);	// tmp test
	return 0;
}

int makeTimestr2(char *tf)
{
	char *url_time = nvram_get("url_time_x_1");
	char *url_date = nvram_get("url_date_x");
	static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int i, comma = 0;

	memset(tf, 0, 256);

	if (!nvram_match("url_enable_x_1", "1"))
		return -1;

	if ((!url_date) || strlen(url_date) != 7 || !strcmp(url_date, "0000000") || !url_time)
	{
		printf("url filter get time fail\n");
		return -1;
	}

	sprintf(tf, "-m time --timestart %c%c:%c%c:00 --timestop %c%c:%c%c:59 --days ", url_time[0], url_time[1], url_time[2], url_time[3], url_time[4], url_time[5], url_time[6], url_time[7]);

	for (i=0; i<7; ++i)
	{
		if (url_date[i] == '1')
		{
			if (comma == 1)
				strncat(tf, ",", 1);

			strncat(tf, days[i], 3);
			comma = 1;
		}
	}

	printf("# url filter time module str is [%s]\n", tf);	// tmp test
	return 0;
}

int
valid_url_filter_time()
{
	char *url_time1 = nvram_get("url_time_x");
	char *url_time2 = nvram_get("url_time_x_1");
	char starttime1[5], endtime1[5];
	char starttime2[5], endtime2[5];

	memset(starttime1, 0, 5);
	memset(endtime1, 0, 5);
	memset(starttime2, 0, 5);
	memset(endtime2, 0, 5);

	if (!nvram_match("url_enable_x", "1") && !nvram_match("url_enable_x_1", "1"))
		return 0;

	if (nvram_match("url_enable_x", "1"))
	{
		if ((!url_time1) || strlen(url_time1) != 8)
			goto err;

		strncpy(starttime1, url_time1, 4);
		strncpy(endtime1, url_time1 + 4, 4);
		printf("starttime1: %s\n", starttime1);
		printf("endtime1: %s\n", endtime1);

		if (atoi(starttime1) >= atoi(endtime1))
			goto err;
	}

	if (nvram_match("url_enable_x_1", "1"))
	{
		if ((!url_time2) || strlen(url_time2) != 8)
			goto err;

		strncpy(starttime2, url_time2, 4);
		strncpy(endtime2, url_time2 + 4, 4);
		printf("starttime2: %s\n", starttime2);
		printf("endtime2: %s\n", endtime2);

		if (atoi(starttime2) >= atoi(endtime2))
			goto err;
	}

	if (nvram_match("url_enable_x", "1") && nvram_match("url_enable_x_1", "1"))
	{
		if ((atoi(starttime1) > atoi(starttime2)) && 
			((atoi(starttime2) > atoi(endtime1)) || (atoi(endtime2) > atoi(endtime1))))
			goto err;

		if ((atoi(starttime2) > atoi(starttime1)) && 
			((atoi(starttime1) > atoi(endtime2)) || (atoi(endtime1) > atoi(endtime2))))
			goto err;
	}

	return 1;

err:
	printf("invalid url filter time setting!\n");
	return 0;
}
/* url filter corss midnight patch end */
#endif

#ifdef CONTENTFILTER
int makeTimestr_content(char *tf)
{
#if 0
	char *keyword_time = nvram_get("keyword_time_x");
	char *keyword_date = nvram_get("keyword_date_x");
#else
	char *keyword_time = nvram_get("url_time_x");
	char *keyword_date = nvram_get("url_date_x");
#endif
	static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int i, comma = 0;

	memset(tf, 0, 256);

#if 0
	if (!nvram_match("keyword_enable_x", "1"))
#else
	if (!nvram_match("url_enable_x", "1"))
#endif
		return -1;

	if ((!keyword_date) || strlen(keyword_date) != 7 || !strcmp(keyword_date, "0000000") || !keyword_time)
	{
		printf("content filter get time fail\n");
		return -1;
	}

	sprintf(tf, "-m time --timestart %c%c:%c%c:00 --timestop %c%c:%c%c:59 --days ", keyword_time[0], keyword_time[1], keyword_time[2], keyword_time[3], keyword_time[4], keyword_time[5], keyword_time[6], keyword_time[7]);

	for (i=0; i<7; ++i)
	{
		if (keyword_date[i] == '1')
		{
			if (comma == 1)
				strncat(tf, ",", 1);

			strncat(tf, days[i], 3);
			comma = 1;
		}
	}

	printf("# content filter time module str is [%s]\n", tf);	// tmp test
	return 0;
}

int makeTimestr2_content(char *tf)
{
#if 0
	char *keyword_time = nvram_get("keyword_time_x_1");
	char *keyword_date = nvram_get("keyword_date_x");
#else
	char *keyword_time = nvram_get("url_time_x_1");
	char *keyword_date = nvram_get("url_date_x");
#endif
	static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int i, comma = 0;

	memset(tf, 0, 256);

#if 0
	if (!nvram_match("keyword_enable_x_1", "1"))
#else
	if (!nvram_match("url_enable_x_1", "1"))
#endif		
		return -1;

	if ((!keyword_date) || strlen(keyword_date) != 7 || !strcmp(keyword_date, "0000000") || !keyword_time)
	{
		printf("content filter get time fail\n");
		return -1;
	}

	sprintf(tf, "-m time --timestart %c%c:%c%c:00 --timestop %c%c:%c%c:59 --days ", keyword_time[0], keyword_time[1], keyword_time[2], keyword_time[3], keyword_time[4], keyword_time[5], keyword_time[6], keyword_time[7]);

	for (i=0; i<7; ++i)
	{
		if (keyword_date[i] == '1')
		{
			if (comma == 1)
				strncat(tf, ",", 1);

			strncat(tf, days[i], 3);
			comma = 1;
		}
	}

	printf("# content filter time module str is [%s]\n", tf);	// tmp test
	return 0;
}

int
valid_keyword_filter_time()
{
#if 0
	char *keyword_time1 = nvram_get("keyword_time_x");
	char *keyword_time2 = nvram_get("keyword_time_x_1");
#else
	char *keyword_time1 = nvram_get("url_time_x");
	char *keyword_time2 = nvram_get("url_time_x_1");
#endif
	char starttime1[5], endtime1[5];
	char starttime2[5], endtime2[5];

	memset(starttime1, 0, 5);
	memset(endtime1, 0, 5);
	memset(starttime2, 0, 5);
	memset(endtime2, 0, 5);

#if 0
	if (!nvram_match("keyword_enable_x", "1") && !nvram_match("keyword_enable_x_1", "1"))
#else
	if (!nvram_match("url_enable_x", "1") && !nvram_match("url_enable_x_1", "1"))
#endif
		return 0;

#if 0
	if (nvram_match("keyword_enable_x", "1"))
#else
	if (nvram_match("url_enable_x", "1"))
#endif
	{
		if ((!keyword_time1) || strlen(keyword_time1) != 8)
			goto err;

		strncpy(starttime1, keyword_time1, 4);
		strncpy(endtime1, keyword_time1 + 4, 4);
		printf("starttime1: %s\n", starttime1);
		printf("endtime1: %s\n", endtime1);

		if (atoi(starttime1) > atoi(endtime1))
			goto err;
	}

#if 0
	if (nvram_match("keyword_enable_x_1", "1"))
#else
	if (nvram_match("url_enable_x_1", "1"))
#endif
	{
		if ((!keyword_time2) || strlen(keyword_time2) != 8)
			goto err;

		strncpy(starttime2, keyword_time2, 4);
		strncpy(endtime2, keyword_time2 + 4, 4);
		printf("starttime2: %s\n", starttime2);
		printf("endtime2: %s\n", endtime2);

		if (atoi(starttime2) > atoi(endtime2))
			goto err;
	}

#if 0
	if (nvram_match("keyword_enable_x", "1") && nvram_match("keyword_enable_x_1", "1"))
#else
	if (nvram_match("url_enable_x", "1") && nvram_match("url_enable_x_1", "1"))
#endif
	{
		if ((atoi(starttime1) > atoi(starttime2)) && 
			((atoi(starttime2) > atoi(endtime1)) || (atoi(endtime2) > atoi(endtime1))))
			goto err;

		if ((atoi(starttime2) > atoi(starttime1)) && 
			((atoi(starttime1) > atoi(endtime2)) || (atoi(endtime1) > atoi(endtime2))))
			goto err;
	}

	return 1;

err:
	printf("invalid content filter time setting!\n");
	return 0;
}
#endif

int
filter_setting(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
{
	FILE *fp;	// oleg patch

	char *proto, *flag, *srcip, *srcport, *dstip, *dstport;
	char *setting, line[256];
	char macaccept[32], chain[3];
	char *ftype, *dtype, *fftype;
	int num;
	int i;
	int wan_port;
//2008.09 magic{
#ifdef WEBSTRFILTER
	char nvname[36], timef[256], timef2[256], *filterstr;
#endif
//2008.09 magic}

	if ((fp=fopen("/tmp/filter_rules", "w"))==NULL) return -1;

	fprintf(fp, "*filter\n:INPUT ACCEPT [0:0]\n:FORWARD ACCEPT [0:0]\n:OUTPUT ACCEPT [0:0]\n:MACS - [0:0]\n:logaccept - [0:0]\n:logdrop - [0:0]\n");

	if ( nvram_match("wan0_proto", "bigpond"))
	{
		fprintf(fp, "-I INPUT -d %s -i %s -p udp --dport %d -j %s\n"
		, nvram_safe_get("wan0_ipaddr")
		, nvram_safe_get("wan0_ifname")
		, 5050
		, "ACCEPT");
	}
	strcpy(macaccept, "");

	if (atoi(nvram_safe_get("macfilter_num_x")) == 0)
		nvram_set("macfilter_enable_x", "0");

	// FILTER from LAN to WAN Source MAC
	if (!nvram_match("macfilter_enable_x", "0"))
	{		
		// LAN/WAN filter		
		g_buf_init();

		if (nvram_match("macfilter_enable_x", "2"))
		{
			dtype = logaccept;
			ftype = logdrop;
			fftype = logdrop;
		}
		else
		{
			dtype = logdrop;
			ftype = logaccept;

			strcpy(macaccept, "MACS");
			fftype = macaccept;
		}
	
		num = atoi(nvram_safe_get("macfilter_num_x"));

		for (i=0;i<num;i++)
		{	
	 		fprintf(fp, "-A INPUT -i %s -m mac --mac-source %s -j %s\n", lan_if, mac_conv("macfilter_list_x", i, line), ftype);
	 		fprintf(fp, "-A FORWARD -i %s -m mac --mac-source %s -j %s\n", lan_if, mac_conv("macfilter_list_x", i, line), fftype);
#if 0
#ifdef GUEST_ACCOUNT			
#ifdef RANGE_EXTENDER
			if (nvram_match("wl_mode_EX", "re"))
			{
				// do nothing
			}
			else
#endif	
			if (nvram_match("wl_guest_ENABLE", "1") && nvram_match("wl_mode_Ex", "ap") && nvram_match("wan_nat_x", "1"))
			{
	 			fprintf(fp, "-A INPUT -i %s -m mac --mac-source %s -j %s\n", WL_GUEST_IF_1, mac_conv("macfilter_list_x", i, line), ftype);
				fprintf(fp, "-A FORWARD -i %s -m mac --mac-source %s -j %s\n", WL_GUEST_IF_1, mac_conv("macfilter_list_x", i, line), fftype);
			}
#endif
#endif	// #if 0
		} 
	} 

	if (!nvram_match("fw_enable_x", "1"))
	{
		if (nvram_match("macfilter_enable_x", "1"))
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -i %s -m state --state NEW -j %s\n"
			,lan_if, logdrop);
#if 0
#ifdef GUEST_ACCOUNT
#ifdef RANGE_EXTENDER
			if (nvram_match("wl_mode_EX", "re"))
			{
				// do nothing
			}
			else
#endif	
			if (nvram_match("wl_guest_ENABLE", "1") && nvram_match("wl_mode_EX", "ap") && nvram_match("wan_nat_x", "1"))
			{
				fprintf(fp, "-I INPUT -i %s -m state --state NEW -j %s\n"
				, WL_GUEST_IF_1, logdrop);
			}
#endif
#endif	// #if 0
		}
	}
	else
	{	
		if (nvram_match("macfilter_enable_x", "1"))
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -m state --state INVALID -j %s\n"
			  "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n"
			  "-A INPUT -i lo -m state --state NEW -j %s\n"
			  "-A INPUT -i %s -m state --state NEW -j %s\n"
			,logdrop, logaccept, "ACCEPT", lan_if, logdrop);
#if 0
#ifdef GUEST_ACCOUNT
#ifdef RANGE_EXTENDER
			if (nvram_match("wl_mode_EX", "re"))
			{
				// do nothing
			}
			else
#endif				
			if (nvram_match("wl_guest_ENABLE", "1") && nvram_match("wl_mode_EX", "ap") && nvram_match("wan_nat_x", "1"))
			{
				fprintf(fp, "-I INPUT -i %s -m state --state NEW -j %s\n"
				, WL_GUEST_IF_1, logdrop);
			}
#endif
#endif	// #if 0
		}
		else
		{
			/* Filter known SPI state */
			fprintf(fp, "-A INPUT -m state --state INVALID -j %s\n"
			  "-A INPUT -m state --state RELATED,ESTABLISHED -j %s\n"
			  "-A INPUT -i lo -m state --state NEW -j %s\n"
			  "-A INPUT -i %s -m state --state NEW -j %s\n"
			,logdrop, logaccept, "ACCEPT", lan_if, "ACCEPT");
#if 0
#ifdef GUEST_ACCOUNT			
#ifdef RANGE_EXTENDER
			if (nvram_match("wl_mode_EX", "re"))
			{
				// do nothing
			}
			else
#endif	
			if (nvram_match("wl_guest_ENABLE", "1") && nvram_match("wl_mode_EX", "ap")  && nvram_match("wan_nat_x", "1"))
			{
				fprintf(fp, "-I INPUT -i %s -m state --state NEW -j %s\n"
				, WL_GUEST_IF_1, "ACCEPT");
			}
#endif
#endif	// #if 0
		}

// oleg patch ~
	/* Pass multicast */
	if (nvram_match("mr_enable_x", "1")) {
		fprintf(fp, "-A INPUT -p 2 -d 224.0.0.0/4 -j %s\n", logaccept);
		fprintf(fp, "-A INPUT -p udp -d 224.0.0.0/4 -j %s\n", logaccept);
	}
// ~ oleg patch
	/* enable incoming packets from broken dhcp servers, which are sending replies
	 * from addresses other than used for query, this could lead to lower level
	 * of security, but it does not work otherwise (conntrack does not work) :-( 
	 */
		//if (nvram_match("wan0_proto", "dhcp") || nvram_match("wan0_proto", "bigpond"))
		if (nvram_match("wan0_proto", "dhcp") || nvram_match("wan0_proto", "bigpond") ||
		    nvram_match("wan_ipaddr", "0.0.0.0"))	// oleg patch
		{
			fprintf(fp, "-A INPUT -p udp --sport 67 --dport 68 -j %s\n", logaccept);
		}
#if 0 
#ifdef GUEST_ACCOUNT			
#ifdef RANGE_EXTENDER
		if (nvram_match("wl_mode_EX", "re"))
		{
			// do nothing
		}
		else
#endif	
		if (nvram_match("wl_guest_ENABLE", "1") && nvram_match("wl_mode_EX", "ap") && nvram_match("wan_nat_x", "1"))
		{
			fprintf(fp, "-I INPUT -i %s -p tcp -m tcp --dport 80 -j %s\n", WL_GUEST_IF_1, logdrop);

			//fprintf(fp, "-I INPUT -i %s -p icmp -j %s\n", WL_GUEST_IF_1, logdrop);
			fprintf(fp, "-I FORWARD -i %s -o br0 -j %s\n", WL_GUEST_IF_1, logdrop);
			fprintf(fp, "-I FORWARD -i br0 -o %s -j %s\n", WL_GUEST_IF_1, logdrop);

#ifdef USB_SUPPORT
			fprintf(fp, "-I INPUT -i %s -p tcp -m tcp --dport %s -j %s\n", WL_GUEST_IF_1, nvram_safe_get("usb_webhttpport_x"), logdrop);

			fprintf(fp, "-I INPUT -i %s -p tcp -m tcp --dport %s -j %s\n", WL_GUEST_IF_1, nvram_safe_get("usb_ftpport_x"), logdrop);
/*
			fprintf(fp, "-I INPUT -i %s -p tcp -m tcp --dport %s -j %s\n", WL_GUEST_IF_1, nvram_safe_get("usb_webactivex_x"), logdrop);
*/
			fprintf(fp, "-I INPUT -i %s -p tcp -m tcp --dport %s -j %s\n", WL_GUEST_IF_1, "515", logdrop);

			fprintf(fp, "-I INPUT -i %s -p tcp -m tcp --dport %s -j %s\n", WL_GUEST_IF_1, "9100", logdrop);

			fprintf(fp, "-I INPUT -i %s -p tcp -m tcp --dport %s -j %s\n", WL_GUEST_IF_1, "3838", logdrop);
#endif
		}
#endif
#endif	// # if 0

		// Firewall between WAN and Local
		if (nvram_match("misc_http_x", "1"))
		{
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport 80 -j %s\n", nvram_safe_get("lan_ipaddr"), logaccept);
		}
#if 0
		if (nvram_match("usb_webenable_x", "2"))
		{
			//fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %s -j %s\n", wan_ip, nvram_safe_get("usb_webhttpport_x"), logaccept);

			//fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %s -j %s\n", wan_ip, nvram_safe_get("usb_webactivex_x"), logaccept);

			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %s -j %s\n", nvram_safe_get("usb_webhttpport_x"), logaccept); // oleg patch
/*
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %s -j %s\n", nvram_safe_get("usb_webactivex_x"), logaccept);  // oleg patch
*/
		}
#endif
		if (!nvram_match("usb_ftpenable_x", "0"))
		{	
			//fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %s -j %s\n", wan_ip, nvram_safe_get("usb_ftpport_x"), logaccept);
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %s -j %s\n", nvram_safe_get("usb_ftpport_x"), logaccept);	// oleg patch
		}

		if (!nvram_match("misc_ping_x", "0"))	// qq
		{
			//fprintf(fp, "-A INPUT -p icmp -d %s -j %s\n", wan_ip, logaccept);
			fprintf(fp, "-A INPUT -p icmp -j %s\n", logaccept);	// oleg patch
		}

		if (!nvram_match("misc_lpr_x", "0"))
		{
/*
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %d -j %s\n", wan_ip, 515, logaccept);
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %d -j %s\n", wan_ip, 9100, logaccept);
			fprintf(fp, "-A INPUT -p tcp -m tcp -d %s --dport %d -j %s\n", wan_ip, 3838, logaccept);
*/
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 515, logaccept);	// oleg patch
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 9100, logaccept);	// oleg patch
			fprintf(fp, "-A INPUT -p tcp -m tcp --dport %d -j %s\n", 3838, logaccept);	// oleg patch
		}

		fprintf(fp, "-A INPUT -j %s\n", logdrop);
	}

/* apps_dm DHT patch */
#if 0
	if (nvram_match("apps_dl_share", "1"))
	{
		fprintf(fp, "-I INPUT -p udp --dport 6881 -j ACCEPT\n");	// DHT port
		// port range
		fprintf(fp, "-I INPUT -p udp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
		fprintf(fp, "-I INPUT -p tcp --dport %s:%s -j ACCEPT\n", nvram_safe_get("apps_dl_share_port_from"), nvram_safe_get("apps_dl_share_port_to"));
	}
#else
	if (apps_running_when_start_firewall)
	{
		fprintf(fp, "-I INPUT -p udp --dport 10000 -j ACCEPT\n");	// DHT port
		// port range
		fprintf(fp, "-I INPUT -p udp --dport %s:%s -j ACCEPT\n", "10001", "10050");
		fprintf(fp, "-I INPUT -p tcp --dport %s:%s -j ACCEPT\n", "10001", "10050");
	}
#endif

// oleg patch ~
	/* Pass multicast */
	if (nvram_match("mr_enable_x", "1"))
	{
		fprintf(fp, "-A FORWARD -p udp -d 224.0.0.0/4 -j ACCEPT\n");
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p udp -d 224.0.0.0/4 -j ACCEPT\n");
	}

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	if (nvram_match("wan_proto", "pptp") || nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "l2tp") ||
	    nvram_match("wan_proto", "3g")
#ifdef CDMA
	 || nvram_match("wan_proto", "cdma")
#endif
	) {
		fprintf(fp, "-A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
	}
// ~ oleg patch
	fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);
	if (strlen(macaccept)>0)
		fprintf(fp, "-A MACS -m state --state ESTABLISHED,RELATED -j %s\n", logaccept);
// ~ oleg patch
	/* Filter out invalid WAN->WAN connections */
	fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", wan_if, lan_if, logdrop); 
		 if (!nvram_match("wan0_ifname", wan_if))
		fprintf(fp, "-A FORWARD -o %s ! -i %s -j %s\n", nvram_safe_get("wan0_ifname"), lan_if, logdrop);
// oleg patch ~
	/* Drop the wrong state, INVALID, packets */
	fprintf(fp, "-A FORWARD -m state --state INVALID -j %s\n", logdrop);
	if (strlen(macaccept)>0)
		fprintf(fp, "-A MACS -m state --state INVALID -j %s\n", logdrop);

	/* Accept the redirect, might be seen as INVALID, packets */
	fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);	
	if (strlen(macaccept)>0)
	{
		fprintf(fp, "-A MACS -i %s -o %s -j %s\n", lan_if, lan_if, logaccept);
#if 0	
#ifdef GUEST_ACCOUNT
#ifdef RANGE_EXTENDER
		if (nvram_match("wl_mode_EX", "re"))
		{
			// do nothing
		}
		else
#endif				
		if (nvram_match("wl_guest_ENABLE", "1") && nvram_match("wl_mode_EX", "ap") && nvram_match("wan_nat_x", "1"))
		{
			//fprintf(fp, "-A MACS -i wl0.1 -o wl0.1 -j %s\n", logaccept);
			fprintf(fp, "-A MACS -i %s -o %s -j %s\n", WL_GUEST_IF_1, WL_GUEST_IF_1, logaccept);
		}
#endif
#endif	// #if 0
	}
	/* Clamp TCP MSS to PMTU of WAN interface */
/* oleg patch mark off
	if ( nvram_match("wan_proto", "pppoe"))
	{
		fprintf(fp, "-I FORWARD -p tcp --tcp-flags SYN,RST SYN -m tcpmss --mss %d: -j TCPMSS "
			  "--set-mss %d\n", atoi(nvram_safe_get("wan_pppoe_mtu"))-39, atoi(nvram_safe_get("wan_pppoe_mtu"))-40);
		
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p tcp --tcp-flags SYN,RST SYN -m tcpmss --mss %d: -j TCPMSS "
			  "--set-mss %d\n", atoi(nvram_safe_get("wan_pppoe_mtu"))-39, atoi(nvram_safe_get("wan_pppoe_mtu"))-40);
	}
	if (nvram_match("wan_proto", "pptp"))
	{
		fprintf(fp, "-A FORWARD -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
		if (strlen(macaccept)>0)
			fprintf(fp, "-A MACS -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu\n");
 	}
*/
	//if (nvram_match("fw_enable_x", "1"))
	
	if ( nvram_match("fw_enable_x", "1") && nvram_match("misc_ping_x", "0") )	// ham 0902 //2008.09 magic
		fprintf(fp, "-A FORWARD -i %s -p icmp -j DROP\n", wan_if);


	if (nvram_match("fw_enable_x", "1") && !nvram_match("fw_dos_x", "0"))	// oleg patch
	{
		// DoS attacks
		// sync-flood protection	
		fprintf(fp, "-A FORWARD -i %s -p tcp --syn -m limit --limit 1/s -j %s\n", wan_if, logaccept);
		// furtive port scanner
		fprintf(fp, "-A FORWARD -i %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j %s\n", wan_if, logaccept);
		// ping of death
		fprintf(fp, "-A FORWARD -i %s -p icmp --icmp-type echo-request -m limit --limit 1/s -j %s\n", wan_if, logaccept);
	}


	// FILTER from LAN to WAN
	// Rules for MAC Filter and LAN to WAN Filter
	// Drop rules always before Accept
	if (nvram_match("macfilter_enable_x", "1"))
		strcpy(chain, "MACS");
	else strcpy(chain, "FORWARD");
		
	if (nvram_match("fw_lw_enable_x", "1"))
	{		
		char lanwan_timematch[128];
		char ptr[32], *icmplist;
		char *ftype, *dtype;

		timematch_conv(lanwan_timematch, "filter_lw_date_x", "filter_lw_time_x");
 
		if (nvram_match("filter_lw_default_x", "DROP"))
		{
			dtype = logdrop;
			ftype = logaccept;

		}
		else
		{
			dtype = logaccept;
			ftype = logdrop;
		}
			
		// LAN/WAN filter		
		g_buf_init();

		foreach_x("filter_lw_num_x")
		{
			proto = protoflag_conv("filter_lw_proto_x", i, 0);
			flag = protoflag_conv("filter_lw_proto_x", i, 1);
			srcip = iprange_ex_conv("filter_lw_srcip_x", i);
			printf("\ncheck srcip = %s\n", srcip);	// tmp test
			srcport = portrange_conv("filter_lw_srcport_x", i);
			dstip = iprange_ex_conv("filter_lw_dstip_x", i);
			dstport = portrange_conv("filter_lw_dstport_x", i);	
			setting=filter_conv(proto, flag, srcip, srcport, dstip, dstport); 
			fprintf(fp, "-A %s %s -i %s -o %s %s -j %s\n", chain, lanwan_timematch, lan_if, wan_if, setting, ftype);
			//if (strcmp(wan_if, "eth3") != 0)	// 0808 ham, it will cause pptp fail
	 		//fprintf(fp, "-A %s %s -i %s -o eth3 %s -j %s\n", chain, lanwan_timematch, lan_if, setting, ftype);
		}

		// ICMP	
		foreach(ptr, nvram_safe_get("filter_lw_icmp_x"), icmplist)
		{
			fprintf(fp, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", chain, lanwan_timematch, lan_if, wan_if, ptr, ftype);
		}	

		// Default
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", chain, lan_if, wan_if, dtype);
	} 
	else if (nvram_match("macfilter_enable_x", "1"))
	{
	 	fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", lan_if, wan_if, logdrop);
	 	fprintf(fp, "-A MACS -i %s -o %s -j %s\n", lan_if, wan_if, logaccept);
	}

	// Block VPN traffic
	if (nvram_match("fw_pt_pptp", "0"))
		fprintf(fp, "-I %s -i %s -o %s -p tcp --dport %d -j %s\n", chain, lan_if, wan_if, 1723, "DROP");
	if (nvram_match("fw_pt_l2tp", "0"))
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 1701, "DROP");
	if (nvram_match("fw_pt_ipsec", "0"))
	{
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 500, "DROP");
		fprintf(fp, "-I %s -i %s -o %s -p udp --dport %d -j %s\n", chain, lan_if, wan_if, 4500, "DROP");
	}
	if (nvram_match("fw_pt_pptp", "0"))
		fprintf(fp, "-I %s -i %s -o %s -p 47 -j %s\n", chain, lan_if, wan_if, "DROP");
	if (nvram_match("fw_pt_ipsec", "0"))
	{
		fprintf(fp, "-I %s -i %s -o %s -p 50 -j %s\n", chain, lan_if, wan_if, "DROP");
		fprintf(fp, "-I %s -i %s -o %s -p 51 -j %s\n", chain, lan_if, wan_if, "DROP");
	}

	// Filter from WAN to LAN
	if (nvram_match("fw_wl_enable_x", "1"))
	{
		char wanlan_timematch[128];
		char ptr[32], *icmplist;
		char *dtype, *ftype;

		timematch_conv(wanlan_timematch, "filter_wl_date_x", "filter_wl_time_x");
		g_buf_init();
	
		if (nvram_match("filter_wl_default_x", "DROP"))
		{
			dtype = logdrop;
			ftype = logaccept;
		}
		else
		{
			dtype = logaccept;
			ftype = logdrop;
		}
			
		foreach_x("filter_wl_num_x")
		{	
			proto = protoflag_conv("filter_wl_proto_x", i, 0);
			flag = protoflag_conv("filter_wl_proto_x", i, 1);
			srcip = iprange_ex_conv("filter_wl_srcip_x", i);
			srcport = portrange_conv("filter_wl_srcport_x", i);
			dstip = iprange_ex_conv("filter_wl_dstip_x", i);
			dstport = portrange_conv("filter_wl_dstport_x", i);	
			setting=filter_conv(proto, flag, srcip, srcport, dstip, dstport);

	 		fprintf(fp, "-A FORWARD %s -i %s -o %s %s -j %s\n", wanlan_timematch, wan_if, lan_if, setting, ftype);
		}

		// ICMP	
		foreach(ptr, nvram_safe_get("filter_wl_icmp_x"), icmplist)
		{
			fprintf(fp, "-A FORWARD %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", wanlan_timematch, wan_if, lan_if, ptr, ftype);
		}	
	 	
		// thanks for Oleg
		// Default
		// fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if, dtype);
	}


	/* Write forward chain rules of NAT */
	//if ((fp1 = fopen("/tmp/nat_forward_rules", "r"))!=NULL)
	// oleg patch ~
	/* Enable Virtual Servers */
	// fprintf(fp, "-A FORWARD -m conntrack --ctstate DNAT -j %s\n", logaccept);	// disable for tmp 

	// add back vts forward rules
	if (nvram_match("wan_nat_x", "1") && nvram_match("vts_enable_x", "1"))
	{
		g_buf_init();
		foreach_x("vts_num_x")
		{
			char *proto;
			char *protono;
			char *port;
			char *lport;
			char *dstip;
			char dstips[32], dstports[12];

			proto = proto_conv("vts_proto_x", i);
			protono = portrange_conv("vts_protono_x", i);
			port = portrange_conv("vts_port_x", i);
			lport = portrange_conv("vts_lport_x", i);
			dstip = ip_conv("vts_ipaddr_x", i);

			if (lport!=NULL && strlen(lport)!=0)
			{
				sprintf(dstips, "%s:%s", dstip, lport);
				sprintf(dstports, "%s", lport);
			}
			else
			{
				sprintf(dstips, "%s:%s", dstip, port);
				sprintf(dstports, "%s", port);
			}

			if (strcmp(proto, "tcp")==0 || strcmp(proto, "both")==0)
			{
				fprintf(fp, "-A FORWARD -p tcp -m tcp -d %s --dport %s -j %s\n",  dstip, dstports, logaccept);  // add back for conntrack patch
			}

			if (strcmp(proto, "udp")==0 || strcmp(proto, "both")==0)
			{
				fprintf(fp, "-A FORWARD -p udp -m udp -d %s --dport %s -j %s\n", dstip, dstports, logaccept);	// add back for conntrack patch
			}

			if (strcmp(proto, "other")==0)
			{
				fprintf(fp, "-A FORWARD -p %s -d %s -j %s\n", protono, dstip, logaccept);	// add back for conntrack patch
			}
		}
	}

	// ~ add back

	if (nvram_match("wan_nat_x", "1") && !nvram_match("sp_battle_ips", "0"))
	// ~ oleg patch

	{
/* oleg patch mark off
		while (fgets(line, sizeof(line), fp1))
		{
			fprintf(fp, "%s", line);
		}

		fclose(fp1);
*/
		fprintf(fp, "-A FORWARD -p udp --dport %d -j %s\n", BASEPORT, logaccept);	// oleg patch
	}

	if (nvram_match("fw_wl_enable_x", "1")) // Thanks for Oleg
	{
		// Default
		fprintf(fp, "-A FORWARD -i %s -o %s -j %s\n", wan_if, lan_if, 
			nvram_match("filter_wl_default_x", "DROP") ? logdrop : logaccept);
	}
	// logaccept chain
	fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logaccept -j ACCEPT\n");

	// logdrop chain
	fprintf(fp,"-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP\" "
		  "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		  "-A logdrop -j DROP\n");

#ifdef WEBSTRFILTER
/* url filter corss midnight patch start */
	if (valid_url_filter_time())
	{
		if (!makeTimestr(timef))
			for (i=0; i<atoi(nvram_safe_get("url_num_x")); i++)
			{
				memset(nvname, 0, 36);
				sprintf(nvname, "url_keyword_x%d", i);
				filterstr =  nvram_safe_get(nvname);

				if (strcmp(filterstr, ""))
//					fprintf(fp,"-A FORWARD -p tcp %s -m webstr --url \"%s\" -j DROP\n", timef, filterstr); //2008.10 magic
					fprintf(fp,"-I FORWARD -p tcp %s -m webstr --url \"%s\" -j DROP\n", timef, filterstr); //2008.10 magic
			}

		if (!makeTimestr2(timef2))
			for (i=0; i<atoi(nvram_safe_get("url_num_x")); i++)
			{
				memset(nvname, 0, 36);
				sprintf(nvname, "url_keyword_x%d", i);
				filterstr =  nvram_safe_get(nvname);

				if (strcmp(filterstr, ""))
					fprintf(fp,"-I FORWARD -p tcp %s -m webstr --url \"%s\" -j DROP\n", timef2, filterstr);
			}
	}
/* url filter corss midnight patch end */
#endif

#ifdef CONTENTFILTER
	if (valid_keyword_filter_time())
	{
		if (!makeTimestr_content(timef))
#if 0
			for (i=0; i<atoi(nvram_safe_get("keyword_num_x")); i++)
#else
			for (i=0; i<atoi(nvram_safe_get("url_num_x")); i++)
#endif
			{
				memset(nvname, 0, 36);
#if 0
				sprintf(nvname, "keyword_keyword_x%d", i);
#else
				sprintf(nvname, "url_keyword_x%d", i);
#endif
				filterstr =  nvram_safe_get(nvname);

				if (strcmp(filterstr, ""))
					fprintf(fp,"-I FORWARD -p tcp --sport 80 %s -m string --string \"%s\" --algo bm -j DROP\n", timef, filterstr);
			}

		if (!makeTimestr2_content(timef2))
#if 0
			for (i=0; i<atoi(nvram_safe_get("keyword_num_x")); i++)
#else
			for (i=0; i<atoi(nvram_safe_get("url_num_x")); i++)
#endif
			{
				memset(nvname, 0, 36);
#if 0
				sprintf(nvname, "keyword_keyword_x%d", i);
#else
				sprintf(nvname, "url_keyword_x%d", i);
#endif
				filterstr =  nvram_safe_get(nvname);

				if (strcmp(filterstr, ""))
					fprintf(fp,"-I FORWARD -p tcp --sport 80 %s -m string --string \"%s\" --algo bm -j DROP\n", timef2, filterstr);
			}
	}
#endif

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	//system("iptables -F");
	system("iptables-restore /tmp/filter_rules");
}

#if 0
int porttrigger_setting()
{
	netconf_app_t apptarget, *app;
	int i;
	char *out_proto, *in_proto, *out_port, *in_port, *desc;
	int  out_start, out_end, in_start, in_end;

	if (!nvram_match("wan_nat_x", "1") ||
	    !nvram_match("autofw_enable_x", "1")) 
		return -1;
	
	g_buf_init();

	foreach_x("autofw_num_x")
	{
		out_proto = proto_conv("autofw_outproto_x", i);
//		out_port = portrange_ex2_conv("autofw_outport_x", i, &out_start, &out_end);
		out_port = portrange_ex2_conv_new("autofw_outport_x", i, &out_start, &out_end);
		in_proto = proto_conv("autofw_inproto_x", i);
		in_port = portrange_ex2_conv("autofw_inport_x", i, &in_start, &in_end);
		desc = general_conv("autofw_desc_x", i);
		app = &apptarget;
		memset(app, 0, sizeof(netconf_app_t));

		/* Parse outbound protocol */
		if (!strncasecmp(out_proto, "tcp", 3))
			app->match.ipproto = IPPROTO_TCP;
		else if (!strncasecmp(out_proto, "udp", 3))
			app->match.ipproto = IPPROTO_UDP;
		else continue;

		/* Parse outbound port range */
		app->match.dst.ports[0] = htons(out_start);
		app->match.dst.ports[1] = htons(out_end);

		/* Parse related protocol */
		if (!strncasecmp(in_proto, "tcp", 3))
			app->proto = IPPROTO_TCP;
		else if (!strncasecmp(in_proto, "udp", 3))
			app->proto = IPPROTO_UDP;
		else continue;

		/* Parse related destination port range */
		app->dport[0] = htons(in_start);
		app->dport[1] = htons(in_end);

		/* Parse mapped destination port range */
		app->to[0] = htons(in_start);
		app->to[1] = htons(in_end);

		/* Parse description */
		if (desc)
			strncpy(app->desc, desc, sizeof(app->desc));

		/* Set interface name (match packets entering LAN interface) */
		strncpy(app->match.in.name, nvram_safe_get("lan_ifname"), IFNAMSIZ);

		/* Set LAN source port range (match packets from any source port) */
		app->match.src.ports[1] = htons(0xffff);

		/* Set target (application specific port forward) */
		app->target = NETCONF_APP;

		if (valid_autofw_port(app))
		{
//			netconf_add_fw((netconf_fw_t *)app);

			/* cmd format:
			 * iptables -t nat -A PREROUTING -i br0 -p INCOMING_PROTOCOL --dport TRIGGER_PORT_FROM(-TRIGGER_PORT_TO) -j autofw --related-proto TRIGGER_PROTOCOL --related-dport INCOMING_PORT_FROM(-INCOMING_PORT_TO) --related-to INCOMING_PORT_FROM(-INCOMING_PORT_TO)
			 *
			 * For example, to set up a trigger for BitTorrent, you'd use this:
			 * iptables -t nat -A PREROUTING -i br0 -p tcp --dport 6881 -j autofw --related-proto tcp --related-dport 6881-6999 --related-to 6881-6999
			 */
			doSystem("iptables -t nat -A PREROUTING -i %s -p %s --dport %s -j autofw --related-proto %s --related-dport %s --related-to %s",
				nvram_safe_get("lan_ifname"),
				out_proto,
				out_port,
				in_proto,
				in_port,
				in_port);
		}
	}
}
#endif

int porttrigger_setting_without_netconf(FILE *fp)
{
	netconf_app_t apptarget, *app;
	int i;
	char *out_proto, *in_proto, *out_port, *in_port, *desc;
	int  out_start, out_end, in_start, in_end;

	if (!nvram_match("autofw_enable_x", "1")) 
		return -1;
	
	g_buf_init();

	foreach_x("autofw_num_x")
	{
		out_proto = proto_conv("autofw_outproto_x", i);
//		out_port = portrange_ex2_conv("autofw_outport_x", i, &out_start, &out_end);
		out_port = portrange_ex2_conv_new("autofw_outport_x", i, &out_start, &out_end);
		in_proto = proto_conv("autofw_inproto_x", i);
		in_port = portrange_ex2_conv("autofw_inport_x", i, &in_start, &in_end);
		desc = general_conv("autofw_desc_x", i);
		app = &apptarget;
		memset(app, 0, sizeof(netconf_app_t));

		/* Parse outbound protocol */
		if (!strncasecmp(out_proto, "tcp", 3))
			app->match.ipproto = IPPROTO_TCP;
		else if (!strncasecmp(out_proto, "udp", 3))
			app->match.ipproto = IPPROTO_UDP;
		else continue;

		/* Parse outbound port range */
		app->match.dst.ports[0] = htons(out_start);
		app->match.dst.ports[1] = htons(out_end);

		/* Parse related protocol */
		if (!strncasecmp(in_proto, "tcp", 3))
			app->proto = IPPROTO_TCP;
		else if (!strncasecmp(in_proto, "udp", 3))
			app->proto = IPPROTO_UDP;
		else continue;

		/* Parse related destination port range */
		app->dport[0] = htons(in_start);
		app->dport[1] = htons(in_end);

		/* Parse mapped destination port range */
		app->to[0] = htons(in_start);
		app->to[1] = htons(in_end);

		/* Parse description */
		if (desc)
			strncpy(app->desc, desc, sizeof(app->desc));

		/* Set interface name (match packets entering LAN interface) */
		strncpy(app->match.in.name, nvram_safe_get("lan_ifname"), IFNAMSIZ);

		/* Set LAN source port range (match packets from any source port) */
		app->match.src.ports[1] = htons(0xffff);

		/* Set target (application specific port forward) */
		app->target = NETCONF_APP;

		if (valid_autofw_port(app))
		{
			/* cmd format:
			 * iptables -t nat -A PREROUTING -i br0 -p INCOMING_PROTOCOL --dport TRIGGER_PORT_FROM(-TRIGGER_PORT_TO) -j autofw --related-proto TRIGGER_PROTOCOL --related-dport INCOMING_PORT_FROM(-INCOMING_PORT_TO) --related-to INCOMING_PORT_FROM(-INCOMING_PORT_TO)
			 *
			 * For example, to set up a trigger for BitTorrent, you'd use this:
			 * iptables -t nat -A PREROUTING -i br0 -p tcp --dport 6881 -j autofw --related-proto tcp --related-dport 6881-6999 --related-to 6881-6999
			 */
/*
			doSystem("iptables -t nat -A PREROUTING -i %s -p %s --dport %s -j autofw --related-proto %s --related-dport %s --related-to %s",
				nvram_safe_get("lan_ifname"),
				out_proto,
				out_port,
				in_proto,
				in_port,
				in_port);
*/
			fprintf(fp, "-A PREROUTING -i %s -p %s --dport %s -j autofw --related-proto %s --related-dport %s --related-to %s\n",
				nvram_safe_get("lan_ifname"),
				out_proto,
				out_port,
				in_proto,
				in_port,
				in_port);
		}
	}
}

int
start_firewall_ex(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip)
{
	DIR *dir;
	struct dirent *file;
	FILE *fp;
	char name[NAME_MAX];
	//char logaccept[32], logdrop[32];
	//oleg patch ~
	char logaccept[32], logdrop[32];
	char *mcast_ifname = nvram_safe_get("wan0_ifname");

	if (is_ap_mode())
		return -1;

	/* mcast needs rp filter to be turned off only for non default iface */
	if (!nvram_match("mr_enable_x", "1") || strcmp(wan_if, mcast_ifname) == 0) 
		mcast_ifname = NULL;
	// ~ oleg patch

	/* Block obviously spoofed IP addresses */
	if (!(dir = opendir("/proc/sys/net/ipv4/conf")))
		perror("/proc/sys/net/ipv4/conf");
	while (dir && (file = readdir(dir))) {
		if (strncmp(file->d_name, ".", NAME_MAX) != 0 &&
		    strncmp(file->d_name, "..", NAME_MAX) != 0) {
			sprintf(name, "/proc/sys/net/ipv4/conf/%s/rp_filter", file->d_name);
			if (!(fp = fopen(name, "r+"))) {
				perror(name);
				break;
			//}
			//fputc('1', fp);
			}
			fputc(mcast_ifname && strncmp(file->d_name, 	// oleg patch
				mcast_ifname, NAME_MAX) == 0 ? '0' : '1', fp);
			fclose(fp);	// oleg patch
		}
	}
	closedir(dir);

	/* Determine the log type */
	if (nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
		strcpy(logaccept, "logaccept");
	else strcpy(logaccept, "ACCEPT");

	if (nvram_match("fw_log_x", "drop") || nvram_match("fw_log_x", "both"))
		strcpy(logdrop, "logdrop");
	else strcpy(logdrop, "DROP");

	apps_running_when_start_firewall = is_apps_running();

	/* nat setting */
	nat_setting(wan_if, wan_ip, lan_if, lan_ip, logaccept, logdrop);

	/* Filter setting */
	filter_setting(wan_if, wan_ip, lan_if, lan_ip, logaccept, logdrop);

	/* Trigger port setting */
//	porttrigger_setting();	// move to nat_setting()

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	if ((fp=fopen("/proc/sys/net/nf_conntrack_max", "w+")))
	{
		if (apps_running_when_start_firewall)
			fputs(MAX_CONNTRACK_DM, fp);
		else
		{
			if (nvram_get("misc_conntrack_x") == NULL)
				fputs(MAX_CONNTRACK_DM, fp);
			else
				fputs(nvram_safe_get("misc_conntrack_x"), fp);
		}

		fclose(fp);
        }
#endif
	if ( (fp=fopen("/proc/sys/net/ipv4/ip_forward", "r+")) ) {
		fputc('1', fp);
		fclose(fp);
	} else
		perror("/proc/sys/net/ipv4/ip_forward");

	/* Tweak NAT performance... */
/*
	if ((fp=fopen("/proc/sys/net/core/netdev_max_backlog", "w+")))
	{
		fputs("2048", fp);
		fclose(fp);
	}
	if ((fp=fopen("/proc/sys/net/core/somaxconn", "w+")))
	{
		fputs("1024", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_max_syn_backlog", "w+")))
	{
		fputs("1024", fp);
		fclose(fp);
	}
	if ((fp=fopen("/proc/sys/net/core/rmem_default", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/core/rmem_max", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/core/wmem_default", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/core/wmem_max", "w+")))
	{
		fputs("262144", fp);
		fclose(fp);
	}
	if ((fp=fopen("/proc/sys/net/ipv4/tcp_rmem", "w+")))
	{
		fputs("8192 131072 262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_wmem", "w+")))
	{
		fputs("8192 131072 262144", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/neigh/default/gc_thresh1", "w+")))
	{
		fputs("1024", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/neigh/default/gc_thresh2", "w+")))
	{
		fputs("2048", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/neigh/default/gc_thresh3", "w+")))
	{
		fputs("4096", fp);
		fclose(fp);
	}
*/
	if ((fp=fopen("/proc/sys/net/ipv4/tcp_fin_timeout", "w+")))
	{
		fputs("40", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_intvl", "w+")))
	{
		fputs("30", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_probes", "w+")))
	{
		fputs("5", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_time", "w+")))
	{
		fputs("1800", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_retries2", "w+")))
	{
		fputs("5", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_syn_retries", "w+")))
	{
		fputs("3", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_synack_retries", "w+")))
	{
		fputs("3", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_tw_recycle", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_tw_reuse", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	/* Tweak DoS-related... */
	if ((fp=fopen("/proc/sys/net/ipv4/icmp_ignore_bogus_error_responses", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/icmp_echo_ignore_broadcasts", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_rfc1337", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_syncookies", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	system("rmmod nf_nat_ftp 2>/dev/null");
	system("rmmod nf_conntrack_ftp 2>/dev/null");

	int port_ftp = atoi(nvram_safe_get("port_ftp"));                
	if ((port_ftp > 0) && (port_ftp < 65536) && (port_ftp != 21))
	{
		doSystem("insmod nf_conntrack_ftp ports=21,%d", port_ftp);
		system("insmod nf_nat_ftp");
	}
	else
	{
		system("insmod nf_conntrack_ftp");
		system("insmod nf_nat_ftp");
	}

	return 0;
}

#if 0
//2008.10 magic{
#ifdef WEBSTRFILTER/* Cherry Cho added in 2007/12/27. */
int web_filter()
{
	netconf_filter_t *rule;
	int i, j, ret = 0, addFilter = 0;
	char nvname[36];
	char *activeDate, *activeTime;
	uint days[2] = {8, 8}; /* days[0]: start  days[1]: end  0: Sunday 1: Monday  6:Saturday*/
	uint activeTimeStart, activeTimeEnd;
	activeDate = nvram_safe_get("url_date_x");
	activeTime = nvram_safe_get("url_time_x");
	activeTimeStart=(((activeTime[0]-'0')*10+(activeTime[1]-'0'))*60 + (activeTime[2]-'0')*10 + (activeTime[3]-'0'))*60;
	activeTimeEnd=(((activeTime[4]-'0')*10+(activeTime[5]-'0'))*60 + (activeTime[6]-'0')*10 + (activeTime[7]-'0'))*60;

	if (nvram_match("url_enable_x", "1")) {
		for (j=0; j<7; j++) {
			if (activeDate[j] == '1') {
				if (days[0]== 8) {
					days[0] = j;
					days[1] = j;
				}
				else
					days[1] = j;
				if (j==6)
					addFilter = 1;
			}
			else if ((activeDate[j] == '0') && days[0]!= 8) {
				addFilter = 1;
			}
			if (addFilter) {
				for (i=0; i<atoi(nvram_safe_get("url_num_x")); i++) {
					memset(rule, 0x0, sizeof(netconf_filter_t));
					if ((days[0]>=0 && days[0]<=6) && (days[1]>=0 && days[1]<=6)) {
						rule->match.days[0] = days[0];
						rule->match.days[1] = days[1];
					}
					if ((activeTimeStart>=0 && activeTimeStart<=(24 * 60 * 60)) &&
					   (activeTimeEnd>=0 && activeTimeEnd<=(24 * 60 * 60))) {
						rule->match.secs[0] = activeTimeStart;
						rule->match.secs[1] = activeTimeEnd;
					}
					memset(nvname, 0x0, 36);
					sprintf(nvname, "url_keyword_x%d", i);
					strcpy(rule->match.module_name, "webstr");
					strncpy(rule->match.webstr_info.string, nvram_safe_get(nvname), 256);
					rule->match.webstr_info.invert = 0;
					rule->match.webstr_info.len = strlen(nvram_safe_get(nvname));
					rule->match.webstr_info.type = NETCONF_WEBSTR_URL;
					rule->target = NETCONF_DROP;
					rule->dir = NETCONF_FORWARD;
					ret = netconf_add_fw((netconf_fw_t *)rule);
				}
				days[0] = 8; /* Reset values of days[0] and days[1] */
				days[1] = 8;
				addFilter = 0;
			}
		}
	}
	return ret;
}

#endif
//2008.10 magic}

portmapping_main(int argc, char *argv[])
{
	char actionname[32], portname[32], ipname[32];
	// Check wan interface
	// argv[1] = Set or Unset	
	// argv[2] = Port
	// argv[3] = IP
	// argv[4] = Item
	sprintf(actionname, "4_MappedAction_%S", argv[4]);
	sprintf(ipname, "4_MappedIP_%s", argv[4]);
	sprintf(portname, "4_MappedInPort_%s", argv[4]); 
	
	if (strcmp(argv[1], "Set")==0)
	{		
		nvram_set(actionname, argv[1]);
		nvram_set(portname, argv[2]);
		nvram_set(ipname, argv[3]);
	}
	else
		nvram_set(actionname, argv[1]);
	
	if (nvram_match("wan_proto", "pppoe"))
		start_firewall_ex("ppp0", "", "br0", "");
	else
	{
		if (nvram_match("wl_mode_EX", "ap"))
			start_firewall_ex("eth0", "", "br0", "");
		else
			start_firewall_ex("eth1", "", "br0", "");
	}
}
#endif

/*
#ifdef USB_SUPPORT
void write_ftp_banip(FILE *fp)
{
	char *ip;
	int i;

	g_buf_init();
			
	foreach_x("usb_bannum_x")
	{
		ip = general_conv("usb_ftpbanip_x", i);
		fprintf(fp, "ban=%s\n", ip);
	}
}


void write_ftp_userlist(FILE *fp)
{
	char *user, *pass, *max, *rights;
	int i, maxuser;
	char passwd[32];
	char dir[64];

	g_buf_init();

	mkdir_if_none("/tmp/harddisk/ftp_pub");
	mkdir_if_none("/tmp/harddisk/ftp_pvt");
		
	foreach_x("usb_ftpnum_x")
	{
		user = general_conv("usb_ftpusername_x", i);
		pass = general_conv("usb_ftppasswd_x", i);
		max = general_conv("usb_ftpmaxuser_x", i);
		rights = general_conv("usb_ftprights_x", i);

		if (strlen(max)==0) maxuser=0;
		else maxuser=atoi(max);

		if (strlen(pass)==0) strcpy(passwd, "*");
		else strcpy(passwd, pass);

		if (strcmp(rights, "Private")==0)
		{
			fprintf(fp, "user=%s %s /ftp_pvt/%s/ %d A\n", user, passwd, user, maxuser);
			sprintf(dir, "/tmp/harddisk/ftp_pvt/%s", user);
			mkdir_if_none(dir);
		}
		else if (strcmp(rights, "Private(WO)")==0)
		{
			fprintf(fp, "user=%s %s /ftp_pvt/%s/ %d U\n", user, passwd, user, maxuser);
			sprintf(dir, "/tmp/harddisk/ftp_pvt/%s", user);
			mkdir_if_none(dir);
		}
		else if (strcmp(rights, "Read/Write/Erase")==0)
			fprintf(fp, "user=%s %s /ftp_pub/ %d A\n", user, passwd, maxuser);
		else if (strcmp(rights, "Read/Write")==0)
			fprintf(fp, "user=%s %s /ftp_pub/ %d DUM\n", user, passwd, maxuser);
		else if (strcmp(rights, "Read Only")==0)
			fprintf(fp, "user=%s %s /ftp_pub/ %d DM\n", user, passwd, maxuser);
		else if (strcmp(rights, "Write Only")==0)
			fprintf(fp, "user=%s %s /ftp_pub/ %d U\n", user, passwd, maxuser);
		else	fprintf(fp, "user=%s %s /ftp_pub/ %d -\n", user, passwd, maxuser);
	}
}
#endif
*/
#endif
