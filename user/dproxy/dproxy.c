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
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>

#include "dproxy.h"
#include "dns_decode.h"
#include "cache.h"
#include "conf.h"
#include "dns_list.h"
#include "dns_construct.h"
#include "dns_io.h"

/*****************************************************************************/
/*****************************************************************************/
int dns_main_quit;
int dns_sock;
fd_set rfds;
dns_request_t *dns_request_list;
/*****************************************************************************/
int is_connected()
{
  FILE *fp;

  if(!config.ppp_detect)return 1;

  fp = fopen( config.ppp_device_file, "r" );
  if(!fp)return 0;
  fclose(fp);
  return 1;
}
#define MAX_RETRY	6
#define DNS_DEBUG	1
#define MAX_DNS_NUM	3	// keep the same as in /tmp/resolv.conf
int retry_dns = 0;
/*****************************************************************************/
int dns_init()
{
  struct sockaddr_in sa;
  struct in_addr ip;
  int retry, ret;

  /* Clear it out */
  memset((void *)&sa, 0, sizeof(sa));

  dns_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  /* Error */
  if( dns_sock < 0 ){
	 debug_perror("Could not create socket");
	perror("dns sock fail");
	 exit(1);
  }

  ip.s_addr = INADDR_ANY;
  sa.sin_family = AF_INET;
  memcpy((void *)&sa.sin_addr, (void *)&ip, sizeof(struct in_addr));
  sa.sin_port = htons(PORT);

  /* bind() the socket to the interface */
  for(retry = 0; retry < MAX_RETRY; ++retry)
  {
  	if ((ret = bind(dns_sock, (struct sockaddr *)&sa, sizeof(struct sockaddr))) < 0){
		debug_perror("dns_init: bind: Could not bind to port");
		perror("dns bind fail");
	 	//exit(1);
	 	sleep(1);
  	} else
		break;
  }
  if(retry == MAX_RETRY)
  {
	printf("**dns try bind retry fail\n");
	exit(1);
  }

  dns_main_quit = 0;

  FD_ZERO( &rfds );
  FD_SET( dns_sock, &rfds );

  dns_request_list = NULL;

  cache_purge( config.purge_time );

  return 1;
}
/*****************************************************************************/
/* This function is added by CMC 8/4/2001 */
void forward_dns_query(dns_request_t *node, dns_request_t *m)
{
  struct in_addr	in;
  FILE			*fp;
  char			line[81], dns_ser_ip[81];
  int 			bad_dns = retry_dns;
  int 			get_dns = 0;

  inet_aton( config.name_server, &in );

  memset(line, 0, sizeof(line));
  memset(dns_ser_ip, 0, sizeof(dns_ser_ip));

  if( (fp = fopen( "/tmp/resolv.conf" , "r")) != NULL) {
	while ( fgets(line, 80, fp) != NULL ){
		if(bad_dns != 0)
		{
			--bad_dns;
			continue;		
		}
		if ( sscanf(line, "nameserver %s", dns_ser_ip) == 1 ){
			get_dns = 1;
			inet_aton( dns_ser_ip, &in );
			/* the first or lastest nameserver */
			//if ( !(node->duplicate_queries & 0x01) )	// tmp test
			break;
		}
  		memset(line, 0, sizeof(line));
  		memset(dns_ser_ip, 0, sizeof(dns_ser_ip));
	}
	fclose(fp);
  }
  if(!get_dns)
  {
	debug("dproxy using default dns server: 8.8.8.8\n");
	memset(dns_ser_ip, 0, sizeof(dns_ser_ip));
	sprintf(dns_ser_ip, "8.8.8.8");		// default google's free dns server
	inet_aton(dns_ser_ip, &in);
  }

  debug("forward_dns_query: query DNS server -- %s\n", inet_ntoa(in) );
  dns_write_packet( dns_sock, in, PORT, m );
}
/*****************************************************************************/
void dns_handle_new_query(dns_request_t *m)
{
  //struct in_addr in;
  int retval = 0;	/* modified by CMC from retval=-1 2002/12/6 */

	debug("handle new query %s\n", m->ip);	// tmp test
  if( m->message.question[0].type == A || m->message.question[0].type == AAA){
	debug("type is %x(A/AAA)\n", m->message.question[0].type);	// tmp test
    /* added by CMC to deny name 2002/11/19 */
    if ( deny_lookup_name( m->cname ) ) {
      debug("%s --> blocked.\n", m->cname);
      dns_construct_error_reply(m);
      dns_write_packet( dns_sock, m->src_addr, m->src_port, m );
      return;
    }
    /* standard query */
    retval = cache_lookup_name( m->cname, m->ip );
  }else if( m->message.question[0].type == PTR ){
	debug("type is %x(PTR)\n", m->message.question[0].type);	// tmp test
    /* reverse lookup */
    retval = cache_lookup_ip( m->ip, m->cname );
  }

  debug(".......... %s ---- %s\n", m->cname, m->ip );

	debug("cache retval is %d\n", retval);	// tmp test
  switch( retval )
    {
    case 0:
      if( is_connected() ){
	debug("Adding to list-> id: %d\n", m->message.header.id);
	dns_request_list = dns_list_add( dns_request_list, m );
	/* relay the query untouched */
	forward_dns_query( dns_request_list, m);  /* modified by CMC 8/3/2001 */
      }else{
	debug("Not connected **\n");
	dns_construct_error_reply(m);
	dns_write_packet( dns_sock, m->src_addr, m->src_port, m );
      }
      break;
    case 1:
      dns_construct_reply( m );
      dns_write_packet( dns_sock, m->src_addr, m->src_port, m );
      debug("Cache hit\n");
      break;
    default:
      debug("Unknown query type: %d\n", m->message.question[0].type );
      debug("CMC: Here is un-reachable code! (2002/12/6)\n");
    }

}
/*****************************************************************************/
void dns_handle_request(dns_request_t *m)
{
  dns_request_t *ptr = NULL;

  /* request may be a new query or a answer from the upstream server */
  ptr = dns_list_find_by_id( dns_request_list, m );

  if( ptr != NULL ){
    debug("Found query in list\n");
    /* message may be a response */
    if( m->message.header.flags.f.question == 1 ){
      //dns_write_packet( dns_sock, ptr->src_addr, ptr->src_port, m );
      debug("Replying with answer from %s\n", inet_ntoa( m->src_addr ));
      if( m->message.header.flags.f.rcode == 0 && /* modified by CMC 2002/12/6 */
          (ptr->message.question[0].type == A || ptr->message.question[0].type == PTR) ){
      	dns_write_packet( dns_sock, ptr->src_addr, ptr->src_port, m );
	debug("Cache append: %s ----> %s\n", m->cname, m->ip );
	cache_name_append( m->cname, m->ip );
      	dns_request_list = dns_list_remove( dns_request_list, ptr );
      }
      else
      {
	debug("failed handle: rcode = %d, type = %d, ignore forwarding\n", m->message.header.flags.f.rcode, ptr->message.question[0].type);	// tmp test
      	//++retry_dns;
	//retry_dns %= MAX_DNS_NUM;
      }
      //dns_request_list = dns_list_remove( dns_request_list, ptr );
    }else{
      ptr->duplicate_queries++;	   /* added by CMC 8/4/2001 */
      debug("Duplicate query(%d)\n", ptr->duplicate_queries);
      if(ptr->duplicate_queries > 0)
      {
      		++retry_dns;
		retry_dns %= MAX_DNS_NUM;
      }
      else
		retry_dns = 0;
      debug("retry_dns:%d\n", retry_dns);
      forward_dns_query( ptr, m); /* added by CMC 8/4/2001 */
    }
  }else{
    dns_handle_new_query( m );
  }

}
/*****************************************************************************/
int dns_main_loop()
{
  struct timeval tv;
  fd_set active_rfds;
  int retval;
  dns_request_t m;
  dns_request_t *ptr, *next;
  //int purge_time = config.purge_time / 60;
  int purge_time = CACHE_CHECK_TIME / DNS_TICK_TIME;	//(30sec) modified by CMC 8/4/2001

  while( !dns_main_quit ){

    /* set the one second time out */
    tv.tv_sec = DNS_TICK_TIME;	  //modified by CMC 8/3/2001
    tv.tv_usec = 0;

    /* now copy the main rfds in the active one as it gets modified by select*/
    active_rfds = rfds;

    retval = select( FD_SETSIZE, &active_rfds, NULL, NULL, &tv );

    if (retval){
      /* data is now available */
      dns_read_packet( dns_sock, &m );
      dns_handle_request( &m );
	debug("handle req done\n");	// tmp test
    }else{
      /* select time out */
      ptr = dns_request_list;
      while( ptr ){
	next = ptr->next;
	ptr->time_pending++;
	if( ptr->time_pending > DNS_TIMEOUT/DNS_TICK_TIME ){
	  /* CMC: ptr->time_pending= DNS_TIMEOUT ~ DNS_TIMEOUT+DNS_TICK_TIME */
	  debug("Request timed out\n");
	  /* send error back */
	  dns_construct_error_reply(ptr);
	  dns_write_packet( dns_sock, ptr->src_addr, ptr->src_port, ptr );
	  dns_request_list = dns_list_remove( dns_request_list, ptr );
	}
	ptr = next;
      } /* while(ptr) */

      /* purge cache */
      purge_time--;
      if( purge_time <= 0 ){			//modified by CMC 8/4/2001
	cache_purge( config.purge_time );
	//purge_time = config.purge_time / 60;
	purge_time = CACHE_CHECK_TIME / DNS_TICK_TIME; 	//(30sec) modified by CMC 8/3/2001
      }

    } /* if (retval) */
  }
  return 0;
}


#if DNS_DEBUG	//added by CMC 8/4/2001
/*****************************************************************************/
void debug_perror( char * msg ) {
	debug( "%s : %s\n" , msg , strerror(errno) );
}
/*****************************************************************************/
void debug(char *fmt, ...)
{
#define MAX_MESG_LEN 1024

  va_list args;
  char text[ MAX_MESG_LEN ];

  sprintf( text, "[ %d ]: ", getpid());
  va_start (args, fmt);
  vsnprintf( &text[strlen(text)], MAX_MESG_LEN - strlen(text), fmt, args);
  va_end (args);

  if( config.debug_file[0] ){
    FILE *fp;
    fp = fopen( config.debug_file, "a");
    if(!fp){
      syslog( LOG_ERR, "could not open log file %m" );
      return;
    }
    fprintf( fp, "%s", text);
    fclose(fp);
  }

  /** if not in daemon-mode stderr was not closed, use it. */
  if( ! config.daemon_mode ) {
    fprintf( stderr, "%s", text);
  }
}
#else
void debug_perror( char * msg ) {}
void debug(char *fmt, ...) {}
#endif
/*****************************************************************************
 * print usage informations to stderr.
 *
 *****************************************************************************/
void usage(char * program , char * message ) {
  fprintf(stderr,"%s\n" , message );
  fprintf(stderr,"usage : %s [-c <config-file>] [-d] [-h] [-P]\n", program );
  fprintf(stderr,"\t-c <config-file>\tread configuration from <config-file>\n");
#if DNS_DEBUG	//added by CMC 8/6/2001
  fprintf(stderr,"\t-d \t\trun in debug (=non-daemon) mode.\n");
#else
  fprintf(stderr,"\t-d \t\trun in debug (=non-daemon) mode.\n");
  fprintf(stderr,"\t-d \t\tCMC: Please re-compile dproxy with DNS_DEBUG=1\n");
#endif
  fprintf(stderr,"\t-P \t\tprint configuration on stdout and exit.\n");
  fprintf(stderr,"\t-h \t\tthis message.\n");
}
/*****************************************************************************
 * get commandline options.
 *
 * @return 0 on success, < 0 on error.
 *****************************************************************************/
int get_options( int argc, char ** argv )
{
  char c = 0;
  int not_daemon = 0, cc;
  int want_printout = 0;
  char * progname = argv[0];

  conf_defaults();

  while( (cc = getopt( argc, argv, "c:dhP")) != EOF ) {
    c = (char)cc;	//added by CMC 8/3/2001
    switch(c) {
	 case 'c':
  		conf_load(optarg);
		break;
	 case 'd':
		not_daemon = 1;
		break;
	 case 'h':
		usage(progname,"");
		return -1;
	 case 'P':
		want_printout = 1;
		break;
	 default:
		usage(progname,"");
		return -1;
    }
  }

  /** unset daemon-mode if -d was given. */
  if( not_daemon ) {
	 config.daemon_mode = 0;
  }

  if( want_printout ) {
	 conf_print();
	 exit(0);
  }
  return 0;
}
/*****************************************************************************/
void sig_hup (int signo)
{
  signal(SIGHUP, sig_hup); /* set this for the next sighup */
  conf_load (config.config_file);
}
/*****************************************************************************/
int main(int argc, char **argv)
{

  /* get commandline options, load config if needed. */
  if(get_options( argc, argv ) < 0 ) {
	  exit(1);
  }

  signal(SIGHUP, sig_hup);

  dns_init();

  if (config.daemon_mode) {
    /* Standard fork and background code */
    switch (fork()) {
	 case -1:	/* Oh shit, something went wrong */
		debug_perror("fork");
		exit(-1);
	 case 0:	/* Child: close off stdout, stdin and stderr */
		close(0);
		//close(1);
		//close(2);
		break;
	 default:	/* Parent: Just exit */
		exit(0);
    }
  }

  dns_main_loop();

  return 0;
}

