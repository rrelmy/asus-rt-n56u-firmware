/*
 * cutter.c	Cut a NATted TCP/IP connection passing through
 * 		an IPTABLES firewall or router.
 *
 * Version	1.03 (experimental)
 * 
 * Date		June 2003
 *
 * Author	Chris Lowth <chris@lowth.com>
 * 
 * Copyright	GNU GENERAL PUBLIC LICENSE Version 2, June 1991
 * 		(see "COPYING" file for details).
 *
 * Docs	& updates
 * 		Can be found at http://www.lowth.com/cutter
 *
 *
 *
 * Help?	This software is free - please consider supporting it's
 * 		development by making your Book, Music or IT purchases
 * 		with Amazon.com or Amazon.co.uk by using the links on
 * 		the www.lowth.com website as your entry to the Amazon
 * 		site. It costs you nothing extra, but Amazon pays the
 * 		author a small commission on any purchases made in this
 * 		manner. Please use one of the following URLs:
 *
 *		  http://www.lowth.com/cutter/uk-shop (UK and europe)
 *		  http://www.lowth.com/cutter/us-shop (America)
 *
 *		If you dont live in the UK or USA, just choose the one
 *		nearest to you.
 *
 * 		I am also seeking input in terms of news of the use of
 * 		this software on new platforms, or ideas for it's
 * 		improvment.
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/socket.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/if_ether.h>
#include <fcntl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <errno.h>

#define ETHHDR	 sizeof(struct ethhdr)
#define TCPHDR	 sizeof(struct tcphdr)
#define IPHDR	 sizeof(struct iphdr)
#define PACKETSIZE  TCPHDR + IPHDR

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 2
#define EXIT_WARNING 1
#define EXIT_SUCCESS 0
#endif

typedef unsigned char uchar;
#define TRUE 1
#define FALSE 0

struct rpack{
	struct iphdr ip;
	struct tcphdr tcp;
	char space[8192];
};

struct tpack{
	struct iphdr ip;
	struct tcphdr tcp;
};

struct pseudo_header{		// pseudo header 4 the checksum
	unsigned source_address;
	unsigned dest_address;
	unsigned char placeholder;
	unsigned char protocol;
	unsigned short tcp_length;
	struct tcphdr tcp;
};

struct ifreq ifreq[32];
struct ifconf ifconf;

char wan_ipaddr[16];

/****************************************************************************
 * Get the IP address of the next hop device for the specified IP (or
 * 0.0.0.0 if there isnt one - ie we are directly connected). And the local
 * interface that we should use to get there.
 */

int getnexthop(in_addr_t ip, char *intf, in_addr_t *gateway)
{
	FILE *id = fopen( "/proc/net/route", "r" );
	char buff[500];
	char iface[32];
	in_addr_t dest_ip, gateway_ip, mask;
	int flags, refcnt, use, metric, mtu, window, irtt;

	if ( id == NULL ) return FALSE;

	while (fgets(buff, sizeof(buff), id) != NULL) {

		memset(iface, 0, sizeof(iface));
		dest_ip = gateway_ip = flags = refcnt = use
			= metric = mask = mtu = window = irtt = -1;

		if (sscanf(buff, "%s %8lx %8lx %4x %d %d %d %8lx %d %d %d",
			iface, &dest_ip, &gateway_ip, &flags, &refcnt,
			&use, &metric, &mask, &mtu, &window, &irtt
		) == 11) {
			dest_ip = dest_ip;
			gateway_ip = gateway_ip;
			mask = mask;
			if (
				iface[0] != '*' &&		// not a rejected interface
				(flags & 0x0001) &&		// route is UP
				(flags & 0x0200) == 0 &&	// not a "reject"
				(ip & mask) == dest_ip		// IP match
			) {
				strcpy(intf, iface);
				*gateway = gateway_ip;
				fclose(id);
				return TRUE;
			}
		}
	}
	fclose(id);
	return FALSE;
}

/****************************************************************************
 * Get the MAC address (in binary format) for a neighbouring IP
 */

int getmac(in_addr_t ip, uchar *mac)
{
	FILE *id = fopen( "/proc/net/arp", "r" );
	union { uchar c[4]; in_addr_t n; } ipu;
	in_addr_t ipn;
	int mac0, mac1, mac2, mac3, mac4, mac5;
	int hwtype, flags;
	char dev[32], mask[32], arpip[32];
	char buff[200];

	if (id == NULL) return FALSE;
	while (fgets(buff, sizeof(buff), id) != NULL) {
		if (sscanf(buff, "%s 0x%x 0x%x %x:%x:%x:%x:%x:%x %s %s",
			arpip,
			&hwtype, &flags,
			&mac0,&mac1,&mac2,&mac3,&mac4,&mac5,
			mask,dev
		) == 11 ) {
			ipn = inet_addr(arpip);
			if (ipn == ip) {
				mac[0]=mac0; mac[1]=mac1; mac[2]=mac2;
				mac[3]=mac3; mac[4]=mac4; mac[5]=mac5;
				fclose(id);
				return TRUE;
			}
		}
	}
	fclose(id);
	return FALSE;
}

unsigned short in_cksum(unsigned short *ptr,int nbytes){

	register long		sum;		// assumes long == 32 bits
	u_short 		oddbyte;
	register u_short	answer;		// assumes u_short == 16 bits

	sum = 0;
	while (nbytes > 1)  {
		sum += *ptr++;
		nbytes -= 2;
	}

	if (nbytes == 1) {
		oddbyte = 0;				 // make sure top half is zero
		*((u_char *) &oddbyte) = *(u_char *)ptr; // one byte only
		sum += oddbyte;
	}

	sum  = (sum >> 16) + (sum & 0xffff);	// add high-16 to low-16
	sum += (sum >> 16);			// add carry
	answer = ~sum;				// ones-complement, then truncate to 16 bits
	return(answer);
}

int getifconfig()
{
	int rtn;
	int raw_sock;

	if ((raw_sock = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) == -1) {
		perror("open packet socket");
		return FALSE;
	}
	memset(ifreq, 0, sizeof(ifreq));
	ifconf.ifc_len = sizeof(ifreq);
	ifconf.ifc_req = ifreq;

	rtn = ioctl(raw_sock, SIOCGIFCONF, &ifconf);

	close(raw_sock);
	return TRUE;
}

int localip(in_addr_t ip)
{
	int i;
	struct sockaddr_in *sa;

	for (i=0; i<ifconf.ifc_len / sizeof(struct ifreq); i++) {
		sa = (struct sockaddr_in *)&(ifreq[i].ifr_addr);
		if ( sa->sin_addr.s_addr == ip)
			return TRUE;
	}
	return FALSE;
}

int send_rst(
	char *from_ip_str,
	u_short fromport,
	char *to_ip_str,
	u_short toport
)
{
	int i_result, raw_sock, rtn;
	in_addr_t gateway_ip;
	struct sockaddr_ll myaddr, hisaddr;
	struct tpack tpack;
	struct rpack rpack;
	struct pseudo_header pheader;
	struct ifreq ifr;
	int ifindex;
	unsigned char mac[6];
	char interf[80];
	int from_ip = inet_addr(from_ip_str);
	int to_ip = inet_addr(to_ip_str);
	time_t tstart;

	printf( "\tsending RST from %s:%d to %s:%d\n", from_ip_str, fromport, to_ip_str, toport);	// J++

	if (!strcmp(from_ip_str, wan_ipaddr))
		return TRUE;

	memset( &tpack, 0, sizeof(tpack) );

	// TCP header
	tpack.tcp.source=htons(fromport);		// 16-bit Source port number
	tpack.tcp.dest=htons(toport);			// 16-bit Destination port
	tpack.tcp.seq=0;				// 32-bit Sequence Number */
	tpack.tcp.ack_seq=0;				// 32-bit Acknowledgement Number */
	tpack.tcp.doff=5;				// Data offset */
	tpack.tcp.res1=0;				// reserved */
	tpack.tcp.urg=0;				// Urgent offset valid flag */
	tpack.tcp.ack=0;				// Acknowledgement field valid flag */
	tpack.tcp.psh=0;				// Push flag */
	tpack.tcp.rst=0;				// Reset flag */
	tpack.tcp.syn=0;				// Synchronize sequence numbers flag */
	tpack.tcp.fin=1;				// Finish sending flag */
	tpack.tcp.window=0;				// 16-bit Window size */
	tpack.tcp.check=0;				// space for 16-bit checksum
	tpack.tcp.urg_ptr=0;				// 16-bit urgent offset */

	//  IP header
	tpack.ip.version=4;				// 4-bit Version */
	tpack.ip.ihl=5; 				// 4-bit Header Length */
	tpack.ip.tos=0x10;				// 8-bit Type of service */
	tpack.ip.tot_len=htons(IPHDR+TCPHDR);		// 16-bit Total length */
	tpack.ip.id=0;					// 16-bit ID field */
	tpack.ip.frag_off=htons(0x4000);		// 13-bit Fragment offset */
	tpack.ip.ttl=0xff;				// 8-bit Time To Live */
	tpack.ip.protocol=IPPROTO_TCP;			// 8-bit Protocol */
	tpack.ip.check=0;				// space for 16-bit Header checksum
	tpack.ip.saddr = from_ip;
	tpack.ip.daddr = to_ip;

	// IP header checksum
	tpack.ip.check=in_cksum((unsigned short *)&tpack.ip,IPHDR);

	// TCP header checksum
	pheader.source_address=(unsigned)tpack.ip.saddr;
	pheader.dest_address=(unsigned)tpack.ip.daddr;
	pheader.placeholder=0;
	pheader.protocol=IPPROTO_TCP;
	pheader.tcp_length=htons(TCPHDR);
	bcopy((char *)&tpack.tcp,(char *)&pheader.tcp,TCPHDR);
	tpack.tcp.check=in_cksum((unsigned short *)&pheader,TCPHDR+12);

	/*
	 * Open a PACKET (layer 2) socket.
	 */

	if ((raw_sock = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) == -1) {
		perror("open packet socket");
		return FALSE;
	}

	if (!getnexthop(to_ip, interf, &gateway_ip)) {
		fprintf(stderr, "Cant find next hop gateway in routing table\n");
		close(raw_sock);
		return FALSE;
	}

	/*
	 * Determine the ifindex of the interface we are going to use with
	 * our layer 2 comms.
	 */

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, interf, sizeof(ifr.ifr_name));
	if (ioctl(raw_sock, SIOCGIFINDEX, &ifr) == -1) {
		perror("ioctl - get ifindex");
		close(raw_sock);
		return FALSE;
	}
	ifindex = ifr.ifr_ifindex;

	if (!getmac( gateway_ip == 0 ? to_ip : gateway_ip, mac) ) {
		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, interf, sizeof(ifr.ifr_name));
		if (ioctl(raw_sock, SIOCGIFHWADDR, &ifr) == -1) {
			perror("ioctl - get hwaddr");
			close(raw_sock);
			return FALSE;
		}
		memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
	}

	/*
	 * Bind to the interface. We can use a null MAC address because the
	 * system will fill it in for us.
	 */

	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sll_family = AF_PACKET;
	myaddr.sll_ifindex = ifindex;
	if(bind(raw_sock, (struct sockaddr*)(&myaddr), sizeof(myaddr)) != 0) {
		perror("bind");
		close(raw_sock);
		return FALSE;
	}

	memset(&hisaddr, 0, sizeof(hisaddr));
	hisaddr.sll_family = AF_PACKET;
	hisaddr.sll_protocol = htons(ETH_P_IP);
	hisaddr.sll_ifindex = ifindex;
	hisaddr.sll_halen = 6;
	memcpy(hisaddr.sll_addr, mac, 6);

	/* Send our invalid FIN packet */

	i_result = sendto(raw_sock,&tpack,PACKETSIZE,0,(void*)&hisaddr,sizeof(hisaddr));
	if (i_result != PACKETSIZE) {
		perror("sendto - fin packet");
		close(raw_sock);
		return FALSE;
	}

	/* Wait for the ACK we expect back from the peer, and send an RST with
	 * the sequence numbers gleaned from the ACK */

	tstart = time(0);

	for ( ; time(0) < tstart + 15; ) {	// give the peer 15 seconds to respond
		struct sockaddr_ll gotaddr;
		int addrlen = sizeof(gotaddr);
		fd_set readfds;
		struct timeval tv;

		FD_ZERO(&readfds);
		FD_SET(raw_sock, &readfds);

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		if (select(raw_sock + 1, &readfds, NULL, NULL, &tv) == 0)
			continue;

		memcpy(&gotaddr, &hisaddr, addrlen);
		memset(&rpack, 0, sizeof(rpack));
		i_result = recvfrom(raw_sock, &rpack, sizeof(rpack), 0, (void*)&gotaddr, &addrlen);
		if (i_result < 16) {
			perror( "recvfrom - waiting for SYN" );
			close(raw_sock);
			return FALSE;
		}
		if (
			rpack.ip.version == 4 &&
			rpack.ip.ihl == 5 &&
			rpack.ip.protocol == IPPROTO_TCP &&
			rpack.ip.saddr == tpack.ip.daddr &&
			rpack.ip.daddr == tpack.ip.saddr &&
			rpack.tcp.source == tpack.tcp.dest &&
			rpack.tcp.dest == tpack.tcp.source &&
			rpack.tcp.ack == 1
		) {
			tpack.tcp.seq = rpack.tcp.ack_seq;
			tpack.tcp.ack_seq = 0;
			tpack.tcp.fin = 0;
			tpack.tcp.rst = 1;
			tpack.tcp.check = 0;

			pheader.source_address=(unsigned)tpack.ip.saddr;
			pheader.dest_address=(unsigned)tpack.ip.daddr;
			pheader.placeholder=0;
			pheader.protocol=IPPROTO_TCP;
			pheader.tcp_length=htons(TCPHDR);
			bcopy((char *)&tpack.tcp,(char *)&pheader.tcp,TCPHDR);
			tpack.tcp.check=in_cksum((unsigned short *)&pheader,TCPHDR+12);

			i_result = sendto(raw_sock,&tpack,PACKETSIZE,0,(void*)&hisaddr,sizeof(hisaddr));
			if (i_result != PACKETSIZE) {
				perror("sendto - reset packet");
				close(raw_sock);
				return FALSE;
			}
			close(raw_sock);
			return TRUE;
		}
	}
//	printf("\t\tACK not seen so RST not sent (sorry!)\n");	// J++
	close(raw_sock);

	return FALSE;
}

int match(in_addr_t match_ip, int match_port, in_addr_t found_ip, int found_port)
{
	if (
		(match_ip == 0 || match_ip == found_ip) &&
		(match_port == 0 || match_port == found_port)
	)
		return TRUE;
	else
		return FALSE;
}

int get_str_field(char **p, char *key, char *out, int outlen)
{
	char *found = strstr(*p, key);
	int i;

	memset(out, 0, outlen);
	if (found == NULL) return FALSE;
	found += strlen(key);
	for (i=0; i<outlen-1 && found[i] && found[i] != ' '; i++) {
		out[i] = found[i];
	}
	out[i] = '\0';
	found += i;
	*p = found;
	return TRUE;
}

int get_int_field(char **p, char *key, int *out)
{
	char buff[80];
	int rtn = get_str_field(p, key, buff, sizeof(buff));
	*out = atoi(buff);
	return rtn;
}

int scan_conntrack(in_addr_t ip1, int port1, in_addr_t ip2, int port2)
{
	FILE *id = fopen( "/proc/net/ip_conntrack", "r" );
	char src1[32], dst1[32], src2[32], dst2[32];
	int sport1, dport1, sport2, dport2, i;
	int packets1, packets2, bytes1, bytes2;
	in_addr_t src1n, src2n, dst1n, dst2n;
	char buff[1024], *p;
	int found = 0;
	int ok = TRUE;

	if (id == NULL) {
		perror( "openning /proc/net/ip_conntrack" );
		return FALSE;
	}

	while (fgets(buff, sizeof(buff), id) != NULL) {
		if (memcmp(buff, "tcp ", 4) != 0 || strstr(buff, " ESTABLISHED ") == NULL)
			continue;

		p = buff;

		if (
			!get_str_field(&p, " src=", src1, sizeof(src1)) ||
			!get_str_field(&p, " dst=", dst1, sizeof(dst1)) ||
			!get_int_field(&p, " sport=", &sport1) ||
			!get_int_field(&p, " dport=", &dport1) ||
			!get_str_field(&p, " src=", src2, sizeof(src2)) ||
			!get_str_field(&p, " dst=", dst2, sizeof(dst2)) ||
			!get_int_field(&p, " sport=", &sport2) ||
			!get_int_field(&p, " dport=", &dport2)
		) continue;

		src1n = inet_addr(src1);
		src2n = inet_addr(src2);
		dst1n = inet_addr(dst1);
		dst2n = inet_addr(dst2);

		if (
			(match(ip1,port1,src2n,sport2) && match(ip2,port2,dst2n,dport2)) ||
			(match(ip1,port1,dst2n,dport2) && match(ip2,port2,src2n,sport2)) ||
			(match(ip1,port1,src1n,sport1) && match(ip2,port2,dst1n,dport1)) ||
			(match(ip1,port1,dst1n,dport1) && match(ip2,port2,src1n,sport1))
		) {
			/*
			 * local network to public network - forwarded connection
			 */

			if (!localip(src1n) && !localip(dst1n) && !localip(src2n) && localip(dst2n)) {
				found ++;
//				printf("For connection %s:%d -> %s:%d\n", src1, sport1, dst1, dport1);	// J++
				if (!strcmp(dst1, wan_ipaddr))						// J++
				ok = send_rst(dst1,dport1,src1,sport1) && ok;
				if (!strcmp(dst2, wan_ipaddr))						// J++
				ok = send_rst(dst2,dport2,src2,sport2) && ok ;
			}

			/* Inbound connection forwarded to private network device */

			else if (!localip(src1n) && localip(dst1n) && !localip(src2n) && !localip(dst2n)) {
				found ++;
//				printf("For connection %s:%d -> %s:%d\n", dst2, dport2, src2, sport2);	// J++
				if (!strcmp(dst1, wan_ipaddr))						// J++
				ok = send_rst(dst1,dport1,src1,sport1) && ok;
				if (!strcmp(dst2, wan_ipaddr))						// J++
				ok = send_rst(dst2,dport2,src2,sport2) && ok;
			}
		}
	}

	if (found == 0) {
		fprintf(stderr, "No matching connections found\n");
		return FALSE;
	}

	return ok;
}

int main(int argc, char *argv[])
{
	in_addr_t ip1 = argc > 1 ? inet_addr(argv[1]) : 0;
	int port1     = argc > 2 ? atoi(argv[2])      : 0;
	in_addr_t ip2 = argc > 3 ? inet_addr(argv[3]) : 0;
	int port2    =  argc > 4 ? atoi(argv[4])      : 0;

	if (argc < 2) {
		fprintf(stderr, "usage: cutter ip [ port [ ip [ port ] ] ]\n");
		exit(EXIT_FAILURE);
	}

	if (ip1 == -1 || ip2 == -1) {
		fprintf(stderr, "Invalid IP address\n");
		exit(EXIT_FAILURE);
	}

	if (argc == 2)
		strcpy(wan_ipaddr, argv[1]);	

	getifconfig();
	if (scan_conntrack(ip1, port1, ip2, port2))
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}
