/* @(#) interface/address conversion utilities */

#ifndef IFADDR_UDPXY_0102081930
#define IFADDR_UDPXY_0102081930

#include <sys/types.h>

#ifdef __cplusplus
 extern "C" {
#endif

struct sockaddr;

/* retrieve IPv4 address of the given network interface
 *
 * @param ifname    name of the network interface
 * @param addr      pointer to socket address structure
 * @param addrlen   socket structure size
 *
 * @return 0 if success, -1 otherwise
 */
int
if2addr( const char* ifname,
         struct sockaddr *addr, size_t addrlen );


/* convert input parameter into an IPv4-address string
 *
 * @param s     input text string
 * @param buf   buffer for the destination string
 * @param len   size of the string buffer
 *
 * @return 0 if success, -1 otherwise
 */
int
get_ipv4_address( const char* s, char* buf, size_t len );


/* split input string into IP address and port
 *
 * @param s     input text string
 * @param addr  IP address (string) destination buffer
 * @param len   buffer length
 * @param port  address of port variable
 *
 * @return 0 in success, !=0 otherwise
 */
int
get_addrport( const char* s, char* addr, size_t len, int* port );


#ifdef __cplusplus
}
#endif

#endif /* IFADDR_UDPXY_0102081930 */

/* __EOF__ */

