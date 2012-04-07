/* vi: set sw=4 ts=4: */
/*
 * arpping.c
 *
 * Mostly stolen from: dhcpcd - DHCP client daemon
 * by Yoichi Hariguchi <yoichi@fore.com>
 */

#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <shutils.h>
#include <nvram/bcmnvram.h>

#define ETHER_ADDR_STR_LEN	18
#define MAC_BCAST_ADDR		(uint8_t *) "\xff\xff\xff\xff\xff\xff"
#define WAN_IF			"eth3"
#define LAN_IF			"br0"

struct arpMsg {
	/* Ethernet header */
	uint8_t  h_dest[6];			/* destination ether addr */
	uint8_t  h_source[6];			/* source ether addr */
	uint16_t h_proto;			/* packet type ID field */

	/* ARP packet */
	uint16_t htype;				/* hardware type (must be ARPHRD_ETHER) */
	uint16_t ptype;				/* protocol type (must be ETH_P_IP) */
	uint8_t  hlen;				/* hardware address length (must be 6) */
	uint8_t  plen;				/* protocol address length (must be 4) */
	uint16_t operation;			/* ARP opcode */
	uint8_t  sHaddr[6];			/* sender's hardware address */
	uint8_t  sInaddr[4];			/* sender's IP address */
	uint8_t  tHaddr[6];			/* target's hardware address */
	uint8_t  tInaddr[4];			/* target's IP address */
	uint8_t  pad[18];			/* pad for min. Ethernet payload (60 bytes) */
} ATTRIBUTE_PACKED;

/* args:	yiaddr - what IP to ping
 *		ip - our ip
 *		mac - our arp address
 *		interface - interface to use
 * retn:	1 addr free
 *		0 addr used
 *		-1 error
 */

//#include <netinet/in.h>
                                                                                                              
static const int one = 1;

int setsockopt_broadcast(int fd)
{
    return setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
}

/* FIXME: match response against chaddr */
int arpping(/*uint32_t yiaddr, uint32_t ip, uint8_t *mac, char *interface*/)
{
	uint32_t yiaddr;
	uint32_t ip;
	uint8_t mac[6];
	char wanmac[18];
	char tmp[3];
	int i, ret;
	char DEV[8];

	if (nvram_match("wan_route_x", "IP_Routed"))
	{
		inet_aton(nvram_safe_get("wan_gateway_t"), &yiaddr);
		inet_aton(get_wan_ipaddr(), &ip);
		strcpy(wanmac, nvram_safe_get("wan0_hwaddr"));	// WAN MAC address
	}
	else
	{
		inet_aton(nvram_safe_get("lan_gateway_t"), &yiaddr);
		inet_aton(get_lan_ipaddr(), &ip);
		strcpy(wanmac, nvram_safe_get("lan_hwaddr"));	// br0 MAC address
	}

	dbg("IP: %s\n", inet_ntoa(ip));
	dbg("Gateway: %s\n", inet_ntoa(yiaddr));
	dbg("MAC: %s\n", wanmac);

        wanmac[17]=0;
        for(i=0;i<6;i++)
        {
                tmp[2]=0;
                strncpy(tmp, wanmac+i*3, 2);
                mac[i]=strtol(tmp, (char **)NULL, 16);
        }

	int	timeout = 2;
	int	s;			/* socket */
	int	rv = 0;			/* return value */
	struct sockaddr addr;		/* for interface name */
	struct arpMsg	arp;
	fd_set		fdset;
	struct timeval	tm;
	time_t		prevTime;


	s = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP));
	if (s == -1) {
		dbg("cannot create raw socket\n");
//		return -1;
//		puts("");
		return 0;
	}

	if (setsockopt_broadcast(s) == -1) {
		dbg("cannot setsocketopt on raw socket\n");
		close(s);
//		return -1;
//		puts("");
		return 0;
	}

	/* send arp request */
	memset(&arp, 0, sizeof(arp));
	memcpy(arp.h_dest, MAC_BCAST_ADDR, 6);		/* MAC DA */
	memcpy(arp.h_source, mac, 6);			/* MAC SA */
	arp.h_proto = htons(ETH_P_ARP);			/* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);		/* hardware type */
	arp.ptype = htons(ETH_P_IP);			/* protocol type (ARP message) */
	arp.hlen = 6;					/* hardware address length */
	arp.plen = 4;					/* protocol address length */
	arp.operation = htons(ARPOP_REQUEST);		/* ARP op code */
	memcpy(arp.sInaddr, &ip, sizeof(ip));		/* source IP address */
	memcpy(arp.sHaddr, mac, 6);			/* source hardware address */
	memcpy(arp.tInaddr, &yiaddr, sizeof(yiaddr));	/* target IP address */

	memset(&addr, 0, sizeof(addr));
	memset(DEV, 0, sizeof(DEV));
	if (nvram_match("wan_route_x", "IP_Routed"))
		strcpy(DEV, WAN_IF);
	else
		strcpy(DEV, LAN_IF);
	strncpy(addr.sa_data, DEV, sizeof(addr.sa_data));

	if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, DEV, IFNAMSIZ) != 0)	// J++
        {
                dbg("setsockopt error: %s\n", DEV);
                perror("setsockopt set:");
        }

	ret = sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr));

        if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, "", IFNAMSIZ) != 0)	// J++
        {
                dbg("setsockopt error: %s\n", "");
                perror("setsockopt reset:");
        }

	if (ret < 0)
	{
		sleep(1);
		return 0;
	}

	/* wait arp reply, and check it */
	tm.tv_usec = 0;
	prevTime = uptime();
	while (timeout > 0) {
		FD_ZERO(&fdset);
		FD_SET(s, &fdset);
		tm.tv_sec = timeout;
		if (select(s + 1, &fdset, (fd_set *) NULL, (fd_set *) NULL, &tm) < 0) {
			dbg("error on ARPING request\n");
			if (errno != EINTR) rv = 0;
		} else if (FD_ISSET(s, &fdset)) {
			if (recv(s, &arp, sizeof(arp), 0) < 0 ) rv = 0;
			if (arp.operation == htons(ARPOP_REPLY) &&
			    memcmp(arp.tHaddr, mac, 6) == 0 &&
			    *((uint32_t *) arp.sInaddr) == yiaddr) {
				dbg("Valid arp reply from [%02X:%02X:%02X:%02X:%02X:%02X]\n",
					(unsigned char)arp.sHaddr[0],
					(unsigned char)arp.sHaddr[1],
					(unsigned char)arp.sHaddr[2],
					(unsigned char)arp.sHaddr[3],
					(unsigned char)arp.sHaddr[4],
					(unsigned char)arp.sHaddr[5]);

				close(s);
				rv = 1;
//				break;
				return 0;
			}
		}
		timeout -= uptime() - prevTime;
		prevTime = uptime();
	}
//	puts("");
	close(s);

	dbg("%salid arp reply\n", rv ? "V" : "No v");
	return rv;
//	return 0;
}
