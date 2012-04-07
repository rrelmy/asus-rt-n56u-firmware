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
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <nvram/bcmnvram.h>
#include <shutils.h>

#include "usb_info.h"
#include "disk_io_tools.h"
#include "disk_initial.h"
#include "disk_share.h"

#define SAMBA_CONF "/etc/smb.conf"

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
#else	/* allow '_' */
	else if (c >= 0x5b && c <= 0x5e)
		ret = 1;
	else if (c == 0x60)
		ret = 1;
#endif
	else if (c >= 0x7b)
		ret = 1;
#if 0
	printf("%c (0x%02x) is %svalid for hostname\n", c, c, (ret == 0) ? "  " : "in");
#endif
	return ret;
}

int
is_valid_hostname(const char *name)
{
	int ret = 1, len, i;

	if (!name)
		return 0;

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
#if 0
	printf("%s is %svalid for hostname\n", name, (ret == 1) ? "  " : "in");
#endif
	return ret;
}

int check_existed_share(const char *string){
	FILE *tp;
	char buf[4096], target[256];

	if((tp = fopen(SAMBA_CONF, "r")) == NULL)
		return 0;

	if(string == NULL || strlen(string) <= 0)
		return 0;

	memset(target, 0, 256);
	sprintf(target, "[%s]", string);

	memset(buf, 0, 4096);
	while(fgets(buf, sizeof(buf), tp) != NULL){
		if(strstr(buf, target)){
			fclose(tp);
			return 1;
		}
	}

	fclose(tp);
	return 0;
}

int main(int argc, char *argv[]) {
	FILE *fp;
	DIR *dir_to_open=NULL;
	int n=0, sh_num=0;
	char *tmp1=NULL;
	char SMB_SHNAME[64];
	char SHNM[16];
	char SMB_SHCOMMENT[64];
	char SHCM[16];
	char SMB_SHPATH[104];
	char SHPH[32];
	char SHAU[16];
	char SMB_SHRRIGHT[384];
	char SHRR[384];
	char SMB_SHWRIGHT[384];
	char SHWR[384];
	char SMB_SHACCUSER[384];
	char *p_computer_name = NULL;
	disk_info_t *follow_disk, *disks_info = NULL;
	partition_info_t *follow_partition;
	
	/* write smb.conf  */
	if ((fp=fopen(SAMBA_CONF, "r"))) {
		fclose(fp);
		unlink(SAMBA_CONF);
	}
	
	if((fp = fopen(SAMBA_CONF, "w")) == NULL)
		goto confpage;
	
	fprintf(fp, "[global]\n");
	if (nvram_safe_get("st_samba_workgroup"))
		fprintf(fp, "workgroup = %s\n", nvram_safe_get("st_samba_workgroup"));

#if 0
	if (nvram_safe_get("computer_name")) {
		fprintf(fp, "netbios name = %s\n", nvram_safe_get("computer_name"));
		fprintf(fp, "server string = %s\n", nvram_safe_get("computer_name"));
	}
#else
	p_computer_name = nvram_get("computer_name") && is_valid_hostname(nvram_get("computer_name")) ? nvram_get("computer_name") : nvram_safe_get("productid");
	if (p_computer_name) {
		fprintf(fp, "netbios name = %s\n", p_computer_name);
		fprintf(fp, "server string = %s\n", p_computer_name);
	}
#endif

	unlink("/var/log.samba");
	fprintf(fp, "log file = /var/log.samba\n");
	fprintf(fp, "log level = 0\n");
	fprintf(fp, "max log size = 5\n");
	
	/* share mode */
	if (!strcmp(nvram_safe_get("st_samba_mode"), "1") || !strcmp(nvram_safe_get("st_samba_mode"), "3")) {
		fprintf(fp, "security = SHARE\n");
		fprintf(fp, "guest ok = yes\n");
		fprintf(fp, "guest only = yes\n");
	}
	else if (!strcmp(nvram_safe_get("st_samba_mode"), "2") || !strcmp(nvram_safe_get("st_samba_mode"), "4")) {
		fprintf(fp, "security = USER\n");
		fprintf(fp, "guest ok = no\n");
		fprintf(fp, "map to guest = Bad User\n");
	}
	else{
		usb_dbg("samba mode: no\n");
		goto confpage;
	}
	
	fprintf(fp, "writeable = yes\n");
	fprintf(fp, "directory mode = 0777\n");
	fprintf(fp, "create mask = 0777\n");
	fprintf(fp, "force directory mode = 0777\n");
	
	/* max users */
	if (strcmp(nvram_safe_get("st_max_user"), "") != 0)
		fprintf(fp, "max connections = %s\n", nvram_safe_get("st_max_user"));
	
	fprintf(fp, "encrypt passwords = yes\n");
	fprintf(fp, "pam password change = no\n");
	fprintf(fp, "socket options = TCP_NODELAY SO_KEEPALIVE SO_RCVBUF=32768 SO_SNDBUF=32768\n");
	fprintf(fp, "obey pam restrictions = no\n");
	fprintf(fp, "use spnego = no\n");		// ASUS add
	fprintf(fp, "client use spnego = no\n");	// ASUS add
	fprintf(fp, "disable spoolss = yes\n");		// ASUS add
	fprintf(fp, "host msdfs = no\n");		// ASUS add
	fprintf(fp, "strict allocate = No\n");		// ASUS add
	fprintf(fp, "null passwords = yes\n");		// ASUS add
//	fprintf(fp, "mangling method = hash2\n");	// ASUS add
	fprintf(fp, "unix charset = UTF8\n");		// ASUS add
	fprintf(fp, "display charset = UTF8\n");	// ASUS add
	fprintf(fp, "bind interfaces only = yes\n");	// ASUS add
	fprintf(fp, "interfaces = lo br0 %s\n", (!nvram_match("sw_mode", "3") ? nvram_safe_get("wan0_ifname") : ""));	// J++
//	fprintf(fp, "dns proxy = no\n");				// J--
	fprintf(fp, "use sendfile = yes\n");

//	fprintf(fp, "domain master = no\n");				// J++
//	fprintf(fp, "wins support = no\n");				// J++
//	fprintf(fp, "printable = no\n");				// J++
//	fprintf(fp, "browseable = yes\n");				// J++
//	fprintf(fp, "security mask = 0777\n");				// J++
//	fprintf(fp, "force security mode = 0\n");			// J++
//	fprintf(fp, "directory security mask = 0777\n");		// J++
//	fprintf(fp, "force directory security mode = 0\n");		// J++

	fprintf(fp, "dos filemode = yes\n");
	fprintf(fp, "dos filetimes = yes\n");
	fprintf(fp, "dos filetime resolution = yes\n");

	disks_info = read_disk_data();
	if (disks_info == NULL) {
		usb_dbg("Couldn't get disk list when writing smb.conf!\n");
		goto confpage;
	}

	/* share */
	if (!strcmp(nvram_safe_get("st_samba_mode"), "0") || !strcmp(nvram_safe_get("st_samba_mode"), "")) {
		;
	}
	else if (!strcmp(nvram_safe_get("st_samba_mode"), "1")) {
		usb_dbg("samba mode: share\n");
		
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				char *mount_folder;
				
				if (follow_partition->mount_point == NULL)
					continue;
				
				mount_folder = strrchr(follow_partition->mount_point, '/')+1;
				
				fprintf(fp, "[%s]\n", mount_folder);
				fprintf(fp, "comment = %s's %s\n", follow_disk->tag, mount_folder);//*/
				/*fprintf(fp, "[%s's part%u]\n", follow_disk->tag, follow_partition->partition_number);
				fprintf(fp, "comment = %s's part%u\n", follow_disk->tag, follow_partition->partition_number);//*/
				fprintf(fp, "path = %s\n", follow_partition->mount_point);
				fprintf(fp, "guest ok = yes\n");
				fprintf(fp, "writeable = yes\n");
//				fprintf(fp, "browseable = yes\n");	// J++
//				fprintf(fp, "hide dot files = no\n");	// J++
				fprintf(fp, "directory mode = 0777\n");
				fprintf(fp, "create mask = 0777\n");

			        fprintf(fp, "map archive = no\n");
			        fprintf(fp, "map hidden = no\n");
			        fprintf(fp, "map read only = no\n");
			        fprintf(fp, "map system = no\n");
			        fprintf(fp, "store dos attributes = yes\n");
			}
		}
	}
	else if (!strcmp(nvram_safe_get("st_samba_mode"), "2")) {
		usb_dbg("samba mode: user\n");
		n = 0;
		sh_num = atoi(nvram_safe_get("sh_num"));
		while (n < sh_num) {
			sprintf(SHPH, "sh_path%d", n);
			sprintf(SHNM, "sh_name%d", n);
			sprintf(SHRR, "sh_rright%d", n);
			sprintf(SHWR, "sh_wright%d", n);
			sprintf(SHCM, "sh_comment%d", n);
			sprintf(SHAU, "sh_acc_user%d", n);
			sprintf(SMB_SHPATH, "%s%s", POOL_MOUNT_ROOT, nvram_safe_get(SHPH));
			sprintf(SMB_SHNAME, "%s", nvram_safe_get(SHNM));
			sprintf(SMB_SHRRIGHT, "%s", nvram_safe_get(SHRR));
			sprintf(SMB_SHWRIGHT, "%s", nvram_safe_get(SHWR));
			sprintf(SMB_SHACCUSER, "%s", nvram_safe_get(SHAU));
			
			while ((tmp1 = strchr(SMB_SHRRIGHT, ';')) != NULL)
				memcpy(tmp1, " ", 1);
			
			memcpy(SMB_SHRRIGHT+strlen(SMB_SHRRIGHT)-1, "\0", 1);
			while ((tmp1=strchr(SMB_SHWRIGHT, ';')) != NULL)
				memcpy(tmp1, " ", 1);
			
			memcpy(SMB_SHWRIGHT+strlen(SMB_SHWRIGHT)-1, "\0", 1);
			while ((tmp1=strchr(SMB_SHACCUSER, ';')) != NULL)
				memcpy(tmp1, " ", 1);
			
			memcpy(SMB_SHACCUSER+strlen(SMB_SHACCUSER)-1, "\0", 1);
			sprintf(SMB_SHCOMMENT, "%s", nvram_safe_get(SHCM));
			
			/*write to conf*/
			if (!strcmp(SMB_SHNAME, "")) {
				goto endloop;
			}
			
			if (!(dir_to_open = opendir(SMB_SHPATH))) {
				goto endloop;
			}
			else
				closedir(dir_to_open);
			
			fprintf(fp, "[%s]\n", SMB_SHNAME);
			fprintf(fp, "comment = %s\n", SMB_SHCOMMENT);
			fprintf(fp, "path = %s\n", SMB_SHPATH);
			if (strstr(SMB_SHWRIGHT, "Guest")) {
				fprintf(fp, "guest ok = yes\n");
			}
			else{
				if (strstr(SMB_SHRRIGHT, "Guest")) {
					fprintf(fp, "guest ok = yes\n");
					fprintf(fp, "writeable = no\n");
					fprintf(fp, "write list = %s\n", SMB_SHWRIGHT);
				}
				else{
					if (!strcmp(SMB_SHWRIGHT, "")&&!strcmp(SMB_SHRRIGHT, ""))
						fprintf(fp, "valid users = _an_si_un_se_shorti_\n");
					else
						fprintf(fp, "valid users = %s\n", SMB_SHACCUSER);
					
					fprintf(fp, "writeable = no\n");
					fprintf(fp, "write list = %s\n", SMB_SHWRIGHT);
					fprintf(fp, "read list = %s\n", SMB_SHRRIGHT);
				}
			}
			
			fprintf(fp, "directory mode = 0777\n");
			fprintf(fp, "create mask = 0777\n");
			
			/*write to conf*/
endloop:
			n++;
		}
	}/* st_samba_mode = 2 */
	else if (!strcmp(nvram_safe_get("st_samba_mode"), "3")) {
		usb_dbg("samba mode: share\n");
		
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				if (follow_partition->mount_point == NULL)
					continue;
				
				char **folder_list;
				
				// 1. get the folder list
				if (get_folder_list_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list) < 0) {
					usb_dbg("Can't read the folder list in %s.\n", follow_partition->mount_point);
					free_2_dimension_list(&sh_num, &folder_list);
					continue;
				}
				
				// 2. start to get every share
				for (n = 0; n < sh_num; ++n) {
					fprintf(fp, "[%s]\n", folder_list[n]);
					fprintf(fp, "comment = %s\n", folder_list[n]);
					fprintf(fp, "path = %s/%s\n", follow_partition->mount_point, folder_list[n]);
					
					fprintf(fp, "writeable = yes\n");
					
					fprintf(fp, "directory mode = 0777\n");
					fprintf(fp, "create mask = 0777\n");
				}
				
				free_2_dimension_list(&sh_num, &folder_list);
			}
		}
	}
	else if (!strcmp(nvram_safe_get("st_samba_mode"), "4")) {
		usb_dbg("samba mode: user\n");
		
		int acc_num;
		char **account_list;
		
		// get the account list
		if (get_account_list(&acc_num, &account_list) < 0) {
			usb_dbg("Can't read the account list.\n");
			free_2_dimension_list(&acc_num, &account_list);
			goto confpage;
		}
			
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				if (follow_partition->mount_point == NULL)
					continue;
				
				char **folder_list;
				
				// 1. get the folder list
				if (get_folder_list_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list) < 0) {
					usb_dbg("Can't read the folder list in %s.\n", follow_partition->mount_point);
					free_2_dimension_list(&sh_num, &folder_list);
					continue;
				}
				
				// 2. start to get every share
				for (n = 0; n < sh_num; ++n) {
					int i, right, first;
					char share[256];
					
					memset(share, 0, 256);
					strcpy(share, folder_list[n]);
					
					fclose(fp);
					
					if(check_existed_share(share)){
						i = 1;
						memset(share, 0, 256);
						sprintf(share, "%s(%d)", folder_list[n], i);
						while(check_existed_share(share)){
							++i;
							memset(share, 0, 256);
							sprintf(share, "%s(%d)", folder_list[n], i);
						}
					}
					
					if((fp = fopen(SAMBA_CONF, "a")) == NULL)
						goto confpage;
					fprintf(fp, "[%s]\n", share);
					fprintf(fp, "comment = %s\n", folder_list[n]);
					fprintf(fp, "path = %s/%s\n", follow_partition->mount_point, folder_list[n]);
					fprintf(fp, "writeable = no\n");
					
					fprintf(fp, "valid users = ");
					first = 1;
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					fprintf(fp, "invalid users = ");
					first = 1;
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (right >= 1)
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					fprintf(fp, "read list = ");
					first = 1;
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (right < 1)
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					fprintf(fp, "write list = ");
					first = 1;
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (right < 2)
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
				}
				
				free_2_dimension_list(&sh_num, &folder_list);
			}
		}
		
		free_2_dimension_list(&acc_num, &account_list);
	}
	
confpage:
	if(fp != NULL)
		fclose(fp);
	free_disk_data(&disks_info);
	return 0;
}
