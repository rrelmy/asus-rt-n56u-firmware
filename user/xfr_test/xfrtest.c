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
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdarg.h>
#include <nvram/bcmnvram.h>
#include <nvram/typedefs.h>
#include <dirent.h>
#include <errno.h>


const char* NLS_NVRAM_U2C="asusnlsu2c";
static char *xfr_buf=NULL;
char tmp[1024];

char *
local2remote(const char *buf)
{
        char *p;
        int i;

        printf("before xfr:\n");
        for(i=0; i<strlen(buf); ++i)
        {
                printf("[%x] ", buf[i]);
        }
	printf("\nlen:(%d)\n", strlen(buf));

	xfr_buf = (char *)malloc(2048);

        memset(tmp, 0, 1024);
        sprintf(tmp, "%s%s_%s", NLS_NVRAM_U2C, "cp950", buf);
        if((p = (char *)nvram_xfr(tmp)) != NULL){
                strcpy(xfr_buf, p);


                printf("after xfr:");
                for(i=0; i<strlen(xfr_buf); ++i)
                {
                        printf("[%x] ", xfr_buf[i]);
                }
                printf("\nlen:(%d)\n", strlen(xfr_buf));

                return xfr_buf;
        }
        else
        {
                return NULL;
        }
}

int
main(int argc, char *argv[])
{
	char *tmp_dirname = NULL;
	DIR *dir;
	struct dirent *dent;

        if(!(dir=opendir("/media/AiDisk_a1")))
        {
                perror("open proc");
                return -1;
        }

        while(dent = readdir(dir))
        {
		tmp_dirname = local2remote(dent->d_name);

		if(!tmp_dirname)
		{
			printf("[xfr result]:[%s](%d)\n", tmp_dirname, strlen(tmp_dirname));	
			free(tmp_dirname);
		}
	}
	
	closedir(dir);
	return 0;
}
