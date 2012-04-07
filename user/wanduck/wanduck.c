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
//#include "typedefs.h"
#include "nvram/typedefs.h"
#include "wanduck.h"
typedef unsigned int __u32;
//#include <ra_ioctl.h>

#include <sys/syscall.h>
_syscall2( int, track_flag, int *, flag, ulong *, ipaddr);

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define csprintf(fmt, args...) do{\
	FILE *cp = fopen("/dev/console", "w");\
	if(cp) {\
		fprintf(cp, fmt, ## args);\
		fclose(cp);\
	}\
}while(0)

#define wan_prefix(unit, prefix) snprintf(prefix, sizeof(prefix), "wan%d_", unit)
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

#define isdigit(c) (c >= '0' && c <= '9') 

#define CHK_PPP		1
#define CHK_DHCPD	2

#define RTL8367M_DEV	"/dev/rtl8367m"

int fer = 0;
int sw_mode = 0;

static long atoid(const char *s)
{
         int i=0,j;
         long k=0;
         for(j=1; j<5; j++) {
                 i=0;
                 while (isdigit(*s)) {
                         i = i*10 + *(s++) - '0';
                }
                 k = k*256 +i;
                 if(j == 4)
                         return k;
                 s++;
        }
         return k;
}

void
track_set(char *c_track)
{
        int k_track = atoi(c_track);
        //ulong ipaddr = atoid(nvram_safe_get("wan_ipaddr_t"));
        ulong ipaddr = 0;

        //printf("track test:[%x, %x]\n", k_track, ipaddr);       // tmp test
        if(!track_flag(&k_track, &ipaddr))
                printf("track set ok\n");
        else
                printf("track set fail\n");
}

static void safe_leave(int signo){

	csprintf("\n## wanduck.safeexit ##\n");
	FD_ZERO(&allset);
	close(http_sock);
	close(dns_sock);
	
	int i;
	for(i = 0; i <  maxfd; ++i)
		close(i);
	
	sleep(1);
	
	if(rule_setup == 1){
		csprintf("\n# Disable direct rule(exit wanduck)\n");
		
		if(sw_mode == 2 )
			system("iptables-restore /tmp/fake_nat_rules");
		else
			_eval(del_command, NULL, 0, NULL);
		
		change_redirect_rules(2, 0);
	}
	
// 2007.11 James {
	char *rm_pid[] = {"rm", "-f", "/var/run/wanduck.pid", NULL};
	
	_eval(rm_pid, NULL, 0, NULL);
// 2007.11 James }
	
csprintf("\n# return(exit wanduck)\n");
	exit(0);
}

// 2010.09 James. {
static void rebuild_rule(int signo){
	if(rule_setup == 1){
csprintf("\n# Rebuild rules by SIGUSR2\n");
		_eval(add_command, NULL, 0, NULL);
		
		change_redirect_rules(1, 0);
	}
}
// 2010.09 James. }

int passivesock(char *service, char *protocol, int qlen){
	//struct servent *pse;
	struct protoent *ppe;
	struct sockaddr_in sin;
	int s, type;
	int protocol_num;
	
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	
	// map service name to port number
	if((sin.sin_port = htons((u_short)atoi(service))) == 0){
		perror("cannot get service entry");
		
		return -1;
	}
	
	// map protocol name to protocol number
	if((ppe = getprotobyname(protocol)) == (struct protoent*)0){
		protocol_num = 0;
		//perror("cannot get proto entry");	// remove for noisy
		//return -1;
	}
	else
		protocol_num = ppe->p_proto;
	
	if(!strcmp(protocol, "udp"))
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;
	
	s = socket(PF_INET, type, protocol_num);
	if(s < 0){
		perror("cannot create socket");
		return -1;
	}
	
	if(bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0){
		perror("cannot bind port");
		close(s);
		
		return -1;
	}
	
	if(type == SOCK_STREAM && listen(s, qlen) < 0){
		perror("cannot listen to port");
		close(s);
		
		return -1;
	}
	
	return s;
}

int chk_gatewayip(char *wanip){
	if(strcmp(wanip, wan_gateway_t) != 0)
		return CONNED;
	else if(strlen(wanip) == 0)
		return CONNED;
	else{
		disconn_case = CASE_MISROUTE;
		
		return DISCONN;
	}
}

int check_ppp_exist(){
	DIR *dir;
	struct dirent *dent;
	char task_file[64], cmdline[64];
	int pid, fd;
	
	if(!(dir = opendir("/proc"))){
		perror("open proc");
		return -1;
	}
	
	while((dent = readdir(dir)) != NULL){
		if((pid = atoi(dent->d_name)) > 1){
			memset(task_file, 0, 64);
			sprintf(task_file, "/proc/%d/cmdline", pid);
			if((fd = open(task_file, O_RDONLY)) > 0){
				memset(cmdline, 0, 64);
				read(fd, cmdline, 64);
				close(fd);
				
				if(strstr(cmdline, "pppd")
						|| strstr(cmdline, "l2tpd")
						){
					closedir(dir);
					return 0;
				}
			}
			else
				printf("cannot open %s\n", task_file);
		}
	}
	
	closedir(dir);
	
	return -1;
}

int file_to_buf(char *path, char *buf, int len){
	FILE *fp;
	memset(buf, 0 , len);
	
	if((fp = fopen(path, "r")) != NULL){
		fgets(buf, len, fp);
		fclose(fp);
		
		return 1;
	}
	
	return 0;
}

char *TASK_TYPE;

unsigned long task_mask;

int
check_task(char *cmd, int target)
{
	if(target == CHK_PPP)
	{
		if(strstr(cmd, "pppd")){
        		TASK_TYPE= "pppd";
                	return 0;
      		}else if(strstr(cmd, "bpalogin")){
                	TASK_TYPE= "bpalogin";
                	return 0;
      		}
	}
	else if(target == CHK_DHCPD)
	{
      		if(strstr(cmd, "udhcpd")){
                	TASK_TYPE= "udhcpd";
                	return 0;
      		}
	}
	else
	{
		TASK_TYPE = "";
        	return -1;
	}
}

/* Find process name by pid from /proc directory */
char *find_name_by_proc(int target){
        DIR  *dir;
        struct dirent *dent;
        char task_file[50], cmdline[64];
        int pid, fd;

        if(!(dir=opendir("/proc")))
        {
                perror("open proc");
                return -1;
        }

        task_mask = 0;
        TASK_TYPE = "";
        while(dent = readdir(dir))
        {
                if((pid=atoi(dent->d_name)) > 1)
                {
                        memset(task_file, 0, sizeof(task_file));
                        sprintf(task_file, "/proc/%d/cmdline", pid);
                        if((fd=open(task_file, O_RDONLY)) > 0)
                        {
                                memset(cmdline, 0, sizeof(cmdline));
                                read(fd, cmdline, sizeof(cmdline));
                                check_task(cmdline, target);
                                close(fd);
                        } else
                                printf("cannot open %s\n", task_file);
                }
        }
        closedir(dir);
        return TASK_TYPE;
}

int
got_wan_ip()
{
	int s;
	struct ifreq ifr;
	struct sockaddr_in *inaddr;
	struct in_addr in_addr;

	if (nvram_match("wan_route_x", "IP_Bridged"))
		return 0;

	/* Retrieve IP info */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return 0;

	if (nvram_match("wan0_proto", "dhcp") || nvram_match("wan0_proto", "static"))
		strncpy(ifr.ifr_name, "eth3", IFNAMSIZ);
	else
		strncpy(ifr.ifr_name, "ppp0", IFNAMSIZ);
	inaddr = (struct sockaddr_in *)&ifr.ifr_addr;
	inet_aton("0.0.0.0", &inaddr->sin_addr);	

	/* Get IP address */
	ioctl(s, SIOCGIFADDR, &ifr);
	close(s);

	struct in_addr ip_addr = ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr;
	if (strcmp("0.0.0.0", inet_ntoa(ip_addr)))
		return 1;
	else
		return 0;
}

int chk_proto(){
	char tmp[100];
	//char filename[80], conntype[10];
	struct ifreq ifr;
	struct sockaddr_in *our_ip;
	struct in_addr in;
	int s;
	FILE *fp;
	char *pwanip = NULL;
	
	/* current unit */
	memset(tmp, 0, 100);
	if(wan_unit == 1)
		strcpy(tmp, wan1_proto);
	else
		strcpy(tmp, wan0_proto);

	if(!strcmp(tmp, "pppoe")
			|| !strcmp(tmp, "pptp")
			|| !strcmp(tmp, "l2tp")
			|| !strcmp(tmp, "3g")
			){
		DIR *ppp_dir;
		struct dirent *entry;
		int got_ppp_link;
		
		if((ppp_dir = opendir("/tmp/ppp")) == NULL){
			disconn_case = CASE_PPPFAIL;
			
			return DISCONN;
		}
		
		got_ppp_link = 0;
		while((entry = readdir(ppp_dir)) != NULL){
			if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
				continue;
			
			if(strstr(entry->d_name, "link") != NULL){
				got_ppp_link = 1;
				
				break;
			}
		}
		closedir(ppp_dir);
		
		if(check_ppp_exist() == -1){
			disconn_case = CASE_PPPFAIL;
			
			return DISCONN;
		}
		else if(got_ppp_link == 0){
			disconn_case = CASE_PPPFAIL;
			
			return DISCONN;
		}
		else if(wan_ready == 0){
			disconn_case = CASE_PPPFAIL;
			
			return DISCONN;
		}
		else
			return CONNED;
	}
	else{
// 2010.09 James. {
		if(wan_ready == 1
				&& !strcmp(lan_subnet_t, wan_subnet_t)
				){
			disconn_case = CASE_THESAMESUBNET;

//			fprintf(stderr, "LAN subnet & WAN subnet conflict!\n");
			if (nvram_match("wan0_proto", "dhcp") || nvram_match("wan0_proto", "static"))
				system("ifconfig eth3 0.0.0.0");
			else
				system("ifconfig ppp0 0.0.0.0");
			
			return DISCONN;
		}
// 2010.09 James. }
		if (got_wan_ip())
			return CONNED;
		else
		{
			disconn_case = CASE_OTHERS;
			return DISCONN;
		}

		/* Open socket to kernel */
#if 0
		if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
			disconn_case = CASE_OTHERS;
			close(s);
			
			return DISCONN;
		}
		else{

			//printf("CONN: case 000\n");	// tmp test
			close(s);
                        return CONNED;
			#if 0
			/* Check for valid IP address */
			memset(tmp, 0, 100);
			if(wan_unit == 1)
				strcpy(tmp, wan1_ifname);
			else
				strcpy(tmp, wan0_ifname);
			
			strncpy(ifr.ifr_name, tmp, IFNAMSIZ);
			if(!ioctl(s, SIOCGIFADDR, &ifr)){
				our_ip = (struct sockaddr_in *)&ifr.ifr_addr;
				in.s_addr = our_ip->sin_addr.s_addr;
				pwanip = inet_ntoa(in);
				close(s);
				
				if( (pwanip == NULL) || (!strcmp(pwanip, "")) ){
					disconn_case = CASE_DISWAN;
					
					printf("DISCONN: case DISWAN 0\n");	// tmp test
					return DISCONN;
				}
				else
					return chk_gatewayip(pwanip);
			}
			else{
				disconn_case = CASE_OTHERS;
				close(s);
				
				printf("DISCONN: case OTHERS 0\n");	// tmp test
				return DISCONN;
			}
			#endif
		}
#endif
	}
}

#if 0
int esw_fd;

int
switch_init(void)
{
        esw_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (esw_fd < 0) {
                perror("socket");
                return -1;
        }
        return 0;
}

void
switch_fini(void)
{
        close(esw_fd);
}

int ra3052_reg_read(int offset, int *value)
{
        struct ifreq ifr;
        esw_reg reg;

        if (value == NULL)
                return -1;
        reg.off = offset;
        strncpy(ifr.ifr_name, "eth2", 5);
        ifr.ifr_data = &reg;
        if (-1 == ioctl(esw_fd, RAETH_ESW_REG_READ, &ifr)) {
                perror("ioctl");
                close(esw_fd);
                return -1;
        }
        *value = reg.val;
        return 0;
}
#endif

int
rtl8367m_wanPort_phyStatus()
{
	int fd;
	int value;

	fd = open(RTL8367M_DEV, O_RDONLY);
	if (fd < 0) {
		perror(RTL8367M_DEV);
		return -1;
	}

	if (ioctl(fd, 0, &value) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}

	close(fd);

	return value;
}

void pre_rSetting()
{
	char *name = find_name_by_proc(CHK_DHCPD);

	if(strlen(name)==0 || strcmp(name, "udhcpd") != 0)
	{
		printf("\nwanduck set r_Setting as 1 (udhcpd dead)\n");	// tmp test
		nvram_set("r_Setting", "1");
	}	
}

#define NOT_SET		0
#define CAT_SET		1
#define HAD_SET		2

int first_enter_repeater()
{
	if(strcmp(nvram_safe_get("sw_mode"), "2")==0)
	{
/*
		char *lan_addr = nvram_safe_get("lan_ipaddr");
		char *lan_addr_t = nvram_safe_get("lan_ipaddr_t");
		
		if(lan_addr_t && lan_addr)
		{
			if((strlen(lan_addr_t)>0) && strcmp(lan_addr_t, lan_addr) != 0)	// assume n13u has gotten ipaddr from wire
			{
				printf("\nwanduck set r_Setting as 1\n");	// tmp test
				nvram_set("r_Setting", "1");
			}
		}
*/
		pre_rSetting();	// set r_Setting when udhcpd is dead

		char *r_setting = nvram_safe_get("r_Setting");

		sw_mode = 2;
		if(strcmp(r_setting, "0")==0)
		{
			if(fer == NOT_SET)
				fer = CAT_SET;
			return 1;
		} 
		else if(strcmp(r_setting, "1")==0)
		{
			fer = HAD_SET;
		}
		else
			return 0;
	}
	else
	{
		sw_mode = 1;
		if(fer != HAD_SET)
			fer = NOT_SET;
		return 0;
	}
}

int if_wan_phyconnected(void)
{
        int val = 0, idx = 1, ret;
#if 0
        if((nvram_match("wan0_proto", "3g")) && (strlen(nvram_safe_get("usb_path1")) > 0) && (strlen(nvram_safe_get("wan0_ipaddr")) > 0))
                return CONNED;

	if(fer < HAD_SET)
	{
		if(first_enter_repeater())
		{
			//printf("First using Repeater\n");	// tmp test
			disconn_case = CASE_FIRST_REPEATER;
			return DISCONN;
		}
	} 

	if(sw_mode == 2)
		return CONNED;

        if(switch_init() < 0)
                return 0;

        if(ra3052_reg_read(0x80, &val) != 0)
                return 0;

        ret = ((val & (idx << 29))) >> 29;

        switch_fini();

//	return ret;
#endif

//	ret = rtl8367m_wanPort_phyStatus();

//	if(!ret){
        if (nvram_match("link_wan", "0")) {
        	disconn_case = CASE_DISWAN;
                return DISCONN;
        }
	else
                return chk_proto();
}

void enable_wan()
{
	if(nvram_match("sw_mode", "1") && nvram_match("wan0_proto", "dhcp"))
	{
		printf("retrieve wan ip\n");	// tmp test
		//system("killall -SIGUSR1 udhcpc");
	}
}

void update_wan(int isup)
{
        if (!isup)
        {
		/*if(nvram_match("wan0_proto", "dhcp"))
		{
                	nvram_set("wan_ipaddr_t", "");
                	nvram_set("wan_gateway_t", "");
                	//nvram_set("wan0_gateway", "");
		}//*/
                nvram_set("wan_status_t", "Disconnected");
        }
        else
        {
                nvram_set("wan_status_t", "Connected");
        }
}

void change_redirect_rules(int num, int force_link_down_up){
	int i;
	char *clean_ip_conntrack[] = {"cat", "/proc/net/nf_conntrack", NULL};
	
	num = 1;	// tmp test
	//nvram_set("wan_state_changed", "1");
	track_set("101");
	// In experience, need to clean the ip_conntrack up in three times for a clean ip_conntrack.
	for(i = 0; i < num; ++i){
csprintf("**** clean ip_conntrack %d time. ****\n", i+1);
		_eval(clean_ip_conntrack, ">/dev/null", 0, NULL);
		
		if(i != num-1)
			sleep(1);
	}
	
	//nvram_set("wan_state_changed", "0");
	track_set("100");

//	system("killall dproxy");
	system("rm -f /tmp/dproxy.cache");
//	system("dproxy -c /tmp/dproxy.conf &");
}

void close_socket(int sockfd, char type){
	close(sockfd);
	FD_CLR(sockfd, &allset);
	client[fd_i].sfd = -1;
	client[fd_i].type = 0;
}

int main(int argc, char **argv){
	char *http_servport, *dns_servport;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	struct timeval  tval;
	int nready, maxi, sockfd, conn_state;
	/*int pid;
	
	// daemonize
	if((pid = fork()) != 0)
		exit(0);//*/
	
	umask(0);
	
	setsid();
	
	chdir("/");
	
	close(STDIN_FILENO);
	//close(STDOUT_FILENO);
	//close(STDERR_FILENO);
	
	struct stat fstatus;
	int fd;
	int max_tbl_sz = getdtablesize();
	for(fd = (STDERR_FILENO+1); fd <= max_tbl_sz; ++fd){
		if(fstat(fd, &fstatus) == 0){
			fprintf(stdout, "The inherited fd(%d) is closed.\n", fd);
			close(fd);
		}
	}
	
	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM, safe_leave);
	signal(SIGUSR1, get_related_nvram2); // 2010.09 James.
	signal(SIGUSR2, rebuild_rule); // 2010.09 James.
	
	if(argc < 3){
		http_servport = DFL_HTTP_SERV_PORT;
		dns_servport = DFL_DNS_SERV_PORT;
	}
	else{
// 2007.10 James {
		if(atoi(argv[1]) <= 0)
			http_servport = DFL_HTTP_SERV_PORT;
		else
			http_servport = argv[1];
		
		if(atoi(argv[2]) <= 0)
			dns_servport = DFL_DNS_SERV_PORT;
		else
			dns_servport = argv[2];
// 2007.10 James }
	}
	
// 2007.10 James {
	if(build_socket(http_servport, dns_servport, &http_sock, &dns_sock) < 0){
		csprintf("\n*** Fail to build socket! ***\n");
		exit(0);
	}
	
	FILE *fp = fopen("/var/run/wanduck.pid", "w");
	
	if(fp != NULL){
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
// 2007.10 James }
	
	maxfd = (http_sock > dns_sock)?http_sock:dns_sock;
	maxi = -1;
	
	tval.tv_sec = 3;
	tval.tv_usec = 0;
	
	FD_ZERO(&allset);
	FD_SET(http_sock, &allset);
	FD_SET(dns_sock, &allset);
	
	for(fd_i = 0; fd_i < MAX_USER; ++fd_i){
		client[fd_i].sfd = -1;
		client[fd_i].type = 0;
	}
	
	rule_setup = 0;
	disconn_case = 0;
	clilen = sizeof(cliaddr);
	
	sleep(3);
	
// 2008.03 James. {
	if(nvram_match("x_Setting", "1"))
		isFirstUse = FALSE;
	else
		isFirstUse = TRUE;
	
	get_related_nvram();
	
	if(nvram_match("wan_ready", "1")){
		wan_ready = 1;
		//nvram_unset("wan_ready");
		get_related_nvram2();
	}
// 2008.03 James. }
	
	err_state = if_wan_phyconnected();
	
	record_conn_status();	// 2008.02 James.
	
	if(err_state == DISCONN){
		if((nat_enable == 1) || (fer < HAD_SET)){
csprintf("\n# Enable direct rule\n");
			rule_setup = 1;
			
			_eval(add_command, NULL, 0, NULL);
			
			change_redirect_rules(2, 0);
		}
	}
	else if(err_state == CONNED && isFirstUse){
		if((nat_enable == 1) || (fer < HAD_SET)){
csprintf("\n#CONNED : Enable direct rule\n");
			rule_setup = 1;
			
			_eval(add_command, NULL, 0, NULL);
			
			change_redirect_rules(2, 0);
		}
	}
	
	for(;;){
		rset = allset;
		tval.tv_sec = 1;
		tval.tv_usec = 0;
		
// 2008.03 James. {
		if(nvram_match("x_Setting", "1"))
			isFirstUse = FALSE;
		else
			isFirstUse = TRUE;
		
		if(wan_ready == 0 && nvram_match("wan_ready", "1")){
			wan_ready = 1;
			//nvram_unset("wan_ready");
			get_related_nvram2();
			
			//rule_setup = 1;	 // disable to check
			
			if(isFirstUse == TRUE)	// 0608 add
			{
csprintf("\n# Rebuild rules\n");
				_eval(add_command, NULL, 0, NULL);
				change_redirect_rules(1, 0);
			}
		}
// 2008.03 James. }
		
		if((nat_enable == 1) || (fer < HAD_SET)){
			conn_state = if_wan_phyconnected();
			
			if(conn_state == CONNED){
				if(err_state == DISCONN)
					err_state = D2C;
			}
			else if(conn_state == DISCONN){
				if(err_state == CONNED)
					err_state = C2D;
			}
			
			record_conn_status();	// 2008.02 James.
			
			if(err_state == C2D || (err_state == CONNED && isFirstUse)){
				err_state = DISCONN;
				
				if(rule_setup == 0){	// 2007.10 James
csprintf("\n# Enable direct rule(C2D)\n");
					rule_setup = 1;
					
					_eval(add_command, NULL, 0, NULL);
					
					change_redirect_rules(2, 1);
					update_wan(0);
				}
			}
			else if(err_state == D2C || err_state == CONNED){
				err_state = CONNED;
				
				if(rule_setup == 1 && !isFirstUse){
csprintf("\n#w Disable direct rule(D2C)\n");
					rule_setup = 0;
					
                			if(sw_mode == 2 )
                        			system("iptables-restore /tmp/fake_nat_rules");
                			else
					{
						enable_wan();
						_eval(del_command, NULL, 0, NULL);
					}
					change_redirect_rules(2, 0);
					update_wan(1);
				}
			}
		}
		else{	// ap mode
			nat_enable = 0;
			
			if(rule_setup == 1){
csprintf("\n#AP Disable direct rule(D2C)\n");
				rule_setup = 0;
				
                		if(sw_mode == 2 )
                        		system("iptables-restore /tmp/fake_nat_rules");
                		else
				{
					enable_wan();
					_eval(del_command, NULL, 0, NULL);
				}

				change_redirect_rules(2, 0);
				update_wan(1);
			}
		}
		
		if((nready = select(maxfd+1, &rset, NULL, NULL, &tval)) <= 0)
			continue;
		
		if(FD_ISSET(dns_sock, &rset)){
			//printf("# run fake dns service\n");	// tmp test
			run_dns_serv(dns_sock);
			if(--nready <= 0)
				continue;
		}
		else if(FD_ISSET(http_sock, &rset)){
			//printf("# run fake httpd service\n");	// tmp test
			if((connfd = accept(http_sock, (struct sockaddr *)&cliaddr, &clilen)) <= 0){
				perror("http accept");
				continue;
			}
			cur_sockfd = connfd;
			
			for(fd_i = 0; fd_i < MAX_USER; ++fd_i){
				if(client[fd_i].sfd < 0){
					client[fd_i].sfd = cur_sockfd;
					client[fd_i].type = T_HTTP;
					break;
				}
			}
			
			if(fd_i == MAX_USER){
csprintf("wanduck servs full\n");
				close(cur_sockfd);
				
				continue;
			}
			
			FD_SET(cur_sockfd, &allset);
			if(cur_sockfd > maxfd)
				maxfd = cur_sockfd;
			if(fd_i > maxi)
				maxi = fd_i;
			
			if(--nready <= 0)
				continue;	// no more readable descriptors
		}
		
		// polling
		for(fd_i = 0; fd_i <= maxi; ++fd_i){
			if((sockfd = client[fd_i].sfd) < 0)
				continue;
			
			if(FD_ISSET(sockfd, &rset)){
// 2007.10 James {
				int nread;
				ioctl(sockfd, FIONREAD, &nread);
				if(nread == 0){
					close_socket(sockfd, T_HTTP);
					continue;
				}
// 2007.10 James }
				
				cur_sockfd = sockfd;
				
				run_http_serv(sockfd);
				
				if(--nready <= 0)
					break;
			}
		}
	}
	
csprintf("wanduck exit error\n");
	exit(1);
}

void run_http_serv(int sockfd){
	ssize_t n;
	char line[MAXLINE];
	
	memset(line, 0, sizeof(line));
	
	if((n = read(sockfd, line, MAXLINE)) == 0){	// client close
		close_socket(sockfd, T_HTTP);
		
		return;
	}
	else if(n < 0){
		perror("readline");
		return;
	}
	else{
		if(client[fd_i].type == T_HTTP)
			handle_http_req(sockfd, line);
		else
			close_socket(sockfd, T_HTTP);
	}
}

void run_dns_serv(int sockfd){
	int n;
	char line[MAXLINE];
	struct sockaddr_in cliaddr;
	int clilen = sizeof(cliaddr);
	
	memset(line, 0, MAXLINE);
	memset(&cliaddr, 0, clilen);
	
	if((n = recvfrom(sockfd, line, MAXLINE, 0, (struct sockaddr *)&cliaddr, &clilen)) == 0)	// client close
		return;
	else if(n < 0){
		perror("readline");
		return;
	}
	else
		handle_dns_req(sockfd, line, n, (struct sockaddr *)&cliaddr, clilen);
}

void parse_dst_url(char *page_src){
	int i, j;
	char dest[STRLEN], host[64];
	char host_strtitle[7], *hp;
	
	j = 0;
	memset(dest, 0, sizeof(dest));
	memset(host, 0, sizeof(host));
	memset(host_strtitle, 0, sizeof(host_strtitle));
	
	for(i = 0; i < strlen(page_src); ++i){
		if(i >= STRLEN)
			break;
		
		if(page_src[i] == ' ' || page_src[i] == '?'){
			dest[j] = '\0';
			break;
		}
		
		dest[j++] = page_src[i];
	}
	
	host_strtitle[0] = '\n';
	host_strtitle[1] = 'H';
	host_strtitle[2] = 'o';
	host_strtitle[3] = 's';
	host_strtitle[4] = 't';
	host_strtitle[5] = ':';
	host_strtitle[6] = ' ';
	
	if((hp = strstr(page_src, host_strtitle)) != NULL){
		hp += 7;
		j = 0;
		for(i = 0; i < strlen(hp); ++i){
			if(i >= 64)
				break;
			
			if(hp[i] == '\r' || hp[i] == '\n'){
				host[j] = '\0';
				break;
			}
			
			host[j++] = hp[i];
		}
	}
	
	memset(dst_url, 0, sizeof(dst_url));
	sprintf(dst_url, "%s/%s", host, dest);
}

void parse_req_queries(char *content, char *lp, int len, int *reply_size){
	int i, rn;
	
	rn = *(reply_size);
	for(i = 0; i < len; ++i){
		content[rn+i] = lp[i];
		if(lp[i] == 0){
			++i;
			break;
		}
	}
	
	if(i >= len)
		return;
	
	content[rn+i] = lp[i];
	content[rn+i+1] = lp[i+1];
	content[rn+i+2] = lp[i+2];
	content[rn+i+3] = lp[i+3];
	i += 4;
	
	*reply_size += i;
}

void handle_http_req(int sfd, char *line){
	int len;
	
	if(!strncmp(line, "GET /", 5)){
		parse_dst_url(line+5);
		
		len = strlen(dst_url);
		if((dst_url[len-4] == '.') &&
				(dst_url[len-3] == 'i') &&
				(dst_url[len-2] == 'c') &&
				(dst_url[len-1] == 'o')){
			close_socket(sfd, T_HTTP);
			
			return;
		}
		send_page(sfd, NULL, dst_url);
	}
	else
		close_socket(sfd, T_HTTP);
}

void handle_dns_req(int sfd, char *line, int maxlen, struct sockaddr *pcliaddr, int clen){
	dns_query_packet d_req;
	dns_response_packet d_reply;
	int reply_size;
	char reply_content[MAXLINE];
	
	reply_size = 0;
	memset(reply_content, 0, MAXLINE);
	memset(&d_req, 0, sizeof(d_req));
	memcpy(&d_req.header, line, sizeof(d_req.header));
	
	// header
	memcpy(&d_reply.header, &d_req.header, sizeof(dns_header));
	//d_reply.header.flag_set.flag_num = htons(0x8580);
	d_reply.header.flag_set.flag_num = htons(0x8180);
	d_reply.header.answer_rrs = htons(0x0001);
	memcpy(reply_content, &d_reply.header, sizeof(d_reply.header));
	reply_size += sizeof(d_reply.header);
	
// 2009.02 James. Force to send answer response.{
	reply_content[5] = 1;	// Questions
	reply_content[7] = 1;	// Answer RRS
	reply_content[9] = 0;	// Authority RRS
	reply_content[11] = 0;	// Additional RRS
// 2009.02 James. }
	
	// queries
	parse_req_queries(reply_content, line+sizeof(dns_header), maxlen-sizeof(dns_header), &reply_size);
	
	// answers
	d_reply.answers.name = htons(0xc00c);
	d_reply.answers.type = htons(0x0001);
	d_reply.answers.ip_class = htons(0x0001);
	//d_reply.answers.ttl = htonl(0x00000001);
	d_reply.answers.ttl = htonl(0x00000000);
	d_reply.answers.data_len = htons(0x0004);
	d_reply.answers.addr = htonl(0x0a000001);	// 10.0.0.1
	
	memcpy(reply_content+reply_size, &d_reply.answers, sizeof(d_reply.answers));
	reply_size += sizeof(d_reply.answers);
	
	sendto(sfd, reply_content, reply_size, 0, pcliaddr, clen);
}

void send_page(int sfd, char *file_dest, char *url){
	char buf[2*MAXLINE];
	time_t now;
	char timebuf[100];
	
	memset(buf, 0, sizeof(buf));
	now = time((time_t*)0);
	(void)strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	
	sprintf(buf, "%s%s%s%s%s%s", buf, "HTTP/1.0 302 Moved Temporarily\r\n", "Server: wanduck\r\n", "Date: ", timebuf, "\r\n");
	
	if(sw_mode == 2 && disconn_case == CASE_FIRST_REPEATER)
		sprintf(buf, "%s%s%s%s%s%s%s" ,buf , "Connection: close\r\n", "Location:http://", lan_ipaddr_t, "/survey.htm", "\r\nContent-Type: text/plain\r\n", "\r\n<html></html>\r\n");
	else if((err_state == C2D || err_state == DISCONN) && disconn_case == CASE_THESAMESUBNET) // 2010.09 James.
		sprintf(buf, "%s%s%s%s%s%d%s%s" ,buf , "Connection: close\r\n", "Location:http://", lan_ipaddr_t, "/error_page.htm?flag=", disconn_case, "\r\nContent-Type: text/plain\r\n", "\r\n<html></html>\r\n");
	else if(isFirstUse)	// 2008.01 James.
		sprintf(buf, "%s%s%s%s%s%s%s" ,buf , "Connection: close\r\n", "Location:http://", lan_ipaddr_t, "/QIS_wizard.htm?flag=detect", "\r\nContent-Type: text/plain\r\n", "\r\n<html></html>\r\n");
	else if(err_state == C2D || disconn_case == CASE_FIRST_REPEATER)
		sprintf(buf, "%s%s%s%s%s%s%s" ,buf , "Connection: close\r\n", "Location:http://", lan_ipaddr_t, "/survey.htm", "\r\nContent-Type: text/plain\r\n", "\r\n<html></html>\r\n");
	else if(err_state == C2D || err_state == DISCONN)
		sprintf(buf, "%s%s%s%s%s%d%s%s" ,buf , "Connection: close\r\n", "Location:http://", lan_ipaddr_t, "/error_page.htm?flag=", disconn_case, "\r\nContent-Type: text/plain\r\n", "\r\n<html></html>\r\n");
	
	write(sfd, buf, strlen(buf));
	close_socket(sfd, T_HTTP);
}

// 2008.02 James. {
void record_conn_status(){
	if(err_state == DISCONN || err_state == C2D){
		if(disconn_case == CASE_DISWAN){
			if(Dr_Surf_case == 1)
				return;
			Dr_Surf_case = 1;
			
			logmessage("WAN Connection", "Ethernet link down.");
		}
		else if(disconn_case == CASE_PPPFAIL){
			if(Dr_Surf_case == 2)
				return;
			Dr_Surf_case = 2;
			
			FILE *fp = fopen("/tmp/wanstatus.log", "r");
			char log_info[64];
			
			if(fp == NULL){
				logmessage("WAN Connection", "WAN was exceptionally disconnected.");
				return;
			}
			
			memset(log_info, 0, 64);
			fgets(log_info, 64, fp);
			fclose(fp);
			
			if(strstr(log_info, "Failed to authenticate ourselves to peer") != NULL)
				logmessage("WAN Connection", "PPPoE or PPTP authentification failed.");
			else
				logmessage("WAN Connection", "No response from the remote server.");
		}
		else if(disconn_case == CASE_DHCPFAIL){
			if(Dr_Surf_case == 3)
				return;
			Dr_Surf_case = 3;
			
			if(!strcmp(wan0_proto, "dhcp"))
				logmessage("WAN Connection", "ISP's DHCP did not function properly.");
			else
				logmessage("WAN Connection", "Detected that the WAN Connection Type was PPPoE. But the PPPoE Setting was not complete.");
		}
		else if(disconn_case == CASE_MISROUTE){
			if(Dr_Surf_case == 4)
				return;
			Dr_Surf_case = 4;
			
			logmessage("WAN Connection", "The router's ip was the same as gateway's ip. It led to your packages couldn't dispatch to internet correctly.");
		}
		else if(disconn_case == CASE_THESAMESUBNET){
			if(Dr_Surf_case == 6)
				return;
			Dr_Surf_case = 6;
			
			logmessage("WAN Connection", "The LAN's subnet may be the same with the WAN's subnet.");
		}
		else{	// disconn_case == CASE_OTHERS
			if(Dr_Surf_case == 5)
				return;
			Dr_Surf_case = 5;
			
			logmessage("WAN Connection", "WAN was exceptionally disconnected.");
		}
	}
	else if(err_state == D2C){
		if(Dr_Surf_case == 10)
			return;
		Dr_Surf_case = 10;
		
		logmessage("WAN Connection", "WAN was restored.");
	}
}

void logmessage(char *logheader, char *fmt, ...){
	va_list args;
	char buf[512];
	
	va_start(args, fmt);
	
	vsnprintf(buf, sizeof(buf), fmt, args);
	openlog(logheader, 0, 0);
	syslog(0, buf);
	closelog();
	va_end(args);
}
// 2008.02 James. }

int readline(int fd,char *ptr,int maxlen){  // read a line(\n, \r\n) each time
	int n,rc;
	char c;
	*ptr = 0;
	
	for(n = 1; n < maxlen; ++n){
		if((rc = read(fd, &c, 1)) == 1){
			*ptr++ = c;
			
			if(c == '\n')
				break;
		}
		else if(rc == 0){
			if(n == 1)
				return(0);
			else
				break;
		}
		else
			return(-1);
	}
	
	return(n);
}

int build_socket(char *http_port, char *dns_port, int *hd, int *dd){
	if((*hd = passivesock(http_port, "tcp", 10)) < 0){
		csprintf("Fail to socket for httpd port: %s.\n", http_port);
		return -1;
	}
	
	if((*dd = passivesock(dns_port, "udp", 10)) < 0){
		csprintf("Fail to socket for DNS port: %s.\n", dns_port);
		return -1;
	}
	
	return 0;
}

void get_related_nvram(){
	memset(wan0_ifname, 0, 16);
	memset(wan1_ifname, 0, 16);
	memset(wan0_proto, 0, 16);
	memset(wan1_proto, 0, 16);
	memset(lan_ipaddr_t, 0, 16);
	
	if(nvram_match("wan_nat_x", "1"))
		nat_enable = 1;
	else
		nat_enable = 0;
	
	wan_unit = atoi(nvram_safe_get("wan_unit"));
	strcpy(wan0_ifname, nvram_safe_get("wan0_ifname"));
	strcpy(wan1_ifname, nvram_safe_get("wan1_ifname"));
	strcpy(wan0_proto, nvram_safe_get("wan0_proto"));
	strcpy(wan1_proto, nvram_safe_get("wan1_proto"));
// 2010.09 James. {
	if(nvram_match("sw_mode", "2"))
		strcpy(lan_ipaddr_t, nvram_safe_get("lan_ipaddr"));
	else
		strcpy(lan_ipaddr_t, nvram_safe_get("lan_ipaddr_t"));
	memset(lan_subnet_t, 0, 11);
	strcpy(lan_subnet_t, nvram_safe_get("lan_subnet_t"));
// 2010.09 James. }
}

void get_related_nvram2(){
	memset(wan_gateway_t, 0, 16);
#if 0
	if (nvram_match("wan0_proto", "dhcp"))
	{
		strcpy(wan_gateway_t, nvram_safe_get("wan_gateway_tmp"));
	}
	else
	{
		strcpy(wan_gateway_t, nvram_safe_get("wan_gateway_t"));
	}
#else
	strcpy(wan_gateway_t, nvram_safe_get("wan_gateway_t"));
#endif

// 2010.09 James. {
	memset(wan_subnet_t, 0, 11);
	strcpy(wan_subnet_t, nvram_safe_get("wan_subnet_t"));
// 2010.09 James. }
}
