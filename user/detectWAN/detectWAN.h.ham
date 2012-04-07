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
/*
 * format of a (udp) probe packet.
 */
struct opacket{
	struct ip ip;
	struct udphdr udp;
	u_char seq;							/* sequence number of this packet */
	u_char ttl;							/* ttl packet left with */
	struct timeval tv;			/* time packet left */
};

#define IPVERSION 4

char *hostname;
struct sockaddr whereto;				/* Who to try to reach */
unsigned int datalen;					/* How much data */

u_char inputpacket[512];						/* last inbound (icmp) packet */
struct opacket *outpacket;			/* last output (udp) packet */

int sendsock = -1;									/* send (udp) socket file descriptor */
int recvsock = -1;									/* receive (icmp) socket file descriptor */

char remote_addr[] = "168.95.1.1";				/* the target address */
int waittime = 1;							/* time to wait for response (in seconds) */
struct timeval wait_timeval;
int MAX_TTL = 5;
int MAX_SEQ = 5;	// tmp test

u_short ident;
u_short port = 32768+666;			/* start udp dest port # for probe packets */

#define CLASS_B_HEAD 2147483648	// 128.0.0.0
#define CLASS_C_HEAD 3221225472	// 192.0.0.0
#define CLASS_D_HEAD 3758096384	// 224.0.0.0
#define CLASS_E_HEAD 4026531840	// 240.0.0.0

#define CLASS_A_PRIVATE_HEAD 167772160	// 10.0.0.0
#define CLASS_A_PRIVATE_TAIL 184549375	// 10.255.255.255
#define CLASS_B_PRIVATE_HEAD 2886729728	// 172.16.0.0
#define CLASS_B_PRIVATE_TAIL 2887778303	// 172.31.255.255
#define CLASS_C_PRIVATE_HEAD 3232235520	// 192.168.0.0
#define CLASS_C_PRIVATE_TAIL 3232301055	// 192.168.255.255

extern int detectWAN();
