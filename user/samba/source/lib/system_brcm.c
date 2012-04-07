/* 
   Unix SMB/CIFS implementation.
   Broadcom implementation of GNU C Library functions used by Samba
   system utilities. 

   Copyright (C) 
   Copyright (C) Broadcom 2004
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
  We've had some issues with the standard system calls, so at least
  temporarily, we've implemented our own versions.
 */

#include "includes.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <system_brcm.h>


#define PASSWD_FILENAME			"/etc/passwd"
#define GROUP_FILENAME			"/etc/group" 


static BOOL
ug_lock( const char * filename, int open_flags, int *file_descriptor )
{
	int result;


	*file_descriptor = open( filename, open_flags );
	if (*file_descriptor == -1)
	{
		DEBUG(0, ("ERROR: open of %s for lock failed with %u: %s.\n", filename, errno, strerror(errno)));
		return False;
	}
	result = flock(*file_descriptor, LOCK_EX);
	if (result == -1)
	{
		DEBUG(0, ("ERROR: lock of %s failed with %u: %s.\n", filename, errno, strerror(errno)));
		close(*file_descriptor);
		return False;
	}
	return True;
}


struct passwd * brcm_getpwnam(const char *name)
{
	int fd;
	static struct passwd result;
	static char line[256];
	FILE *fp;
	char *tok;

	if (name == NULL || *name == '\0')
		return NULL;
	
	//DEBUG(1, ("entered name=%s\n", name));

	if (!ug_lock(PASSWD_FILENAME, O_RDONLY, &fd))
	{
		return NULL;
	}
	
	if ((fp = fdopen(fd, "r")) == NULL)
	{
		DEBUG(1, ("open failed, errno=%u, str='%s'\n", errno, strerror(errno)));
		close(fd);
		return NULL;	
	}
	
	while (fgets(line, 256, fp) != NULL)
	{
		char *ptr = line;
		//DEBUG(1, ("line='%s'\n", line));
	
		tok = strsep( &ptr, ":" );
		if (tok == NULL)
			return NULL;
		if (strcmp(name, tok) == 0)
		{
			/* Yeah, we found it.  Parse the rest. */
			result.pw_name		= tok;
			result.pw_passwd	= strsep( &ptr, ":" );
			result.pw_uid		= strtol( strsep( &ptr, ":" ), NULL, 10 );
			result.pw_gid		= strtol( strsep( &ptr, ":" ), NULL, 10 );
			result.pw_gecos		= strsep( &ptr, ":" );
			result.pw_dir		= strsep( &ptr, ":" );
			result.pw_shell		= strsep( &ptr, ":" );

			//DEBUG(1, ("match! %s:%s:%u:%u:%s:%s:%s\n", result.pw_name,result.pw_passwd,result.pw_uid,result.pw_gid,result.pw_gecos,result.pw_dir,result.pw_shell));
			
			fclose( fp );
			return &result;
		}
	}
	fclose( fp );
	return NULL;	
}


struct passwd * brcm_getpwuid(uid_t uid)
{
	int fd;
	static struct passwd result;
	static char line[256];
	FILE *fp;
	char *tok;
	
	//DEBUG(1, ("gew_getpwuid:1: entered uid=%u\n",uid));
	
	if (!ug_lock(PASSWD_FILENAME, O_RDONLY, &fd))
	{
		return NULL;
	}
	
	if ((fp = fdopen(fd, "r")) == NULL)
	{
		DEBUG(1, ("gew_getpwuid:1: open failed, errno=%u, str='%s'\n", errno, strerror(errno)));
		close(fd);
		return NULL;	
	}
	
	while (fgets(line, 256, fp) != NULL)
	{
		char *ptr = line;
		//DEBUG(1, ("gew_getpwuid:1: line='%s'\n", line));
	
		tok = strsep( &ptr, ":" );
		if (tok == NULL)
			return NULL;
			
		/* Use 'result' as a temporary. */
		result.pw_name		= tok;
		result.pw_passwd	= strsep( &ptr, ":" );
		result.pw_uid		= strtol( strsep( &ptr, ":" ), NULL, 10 );
		result.pw_gid		= strtol( strsep( &ptr, ":" ), NULL, 10 );
		result.pw_gecos		= strsep( &ptr, ":" );
		result.pw_dir		= strsep( &ptr, ":" );
		result.pw_shell		= strsep( &ptr, ":" );

		if (uid == result.pw_uid)
		{
			//DEBUG(1, ("gew_getpwuid:1: match! %s:%s:%u:%u:%s:%s:%s\n", result.pw_name,result.pw_passwd,result.pw_uid,result.pw_gid,result.pw_gecos,result.pw_dir,result.pw_shell));
			
			fclose( fp );
			return &result;
		}
	}
	fclose( fp );
	return NULL;	
}

static int brcm_pw_fd = -1;

void brcm_setpwent(void)
{
        if (brcm_pw_fd != -1)
		close(brcm_pw_fd);

	brcm_pw_fd = open(GROUP_FILENAME, O_RDONLY);  
}

void brcm_endpwent(void)
{
	if (brcm_pw_fd != -1)
		close(brcm_pw_fd);
	brcm_pw_fd = -1;
}

struct passwd *brcm_getpwent(void)
{
	if (brcm_pw_fd != -1)
		return (brcm__getpwent(brcm_pw_fd));
	return NULL;
}



#if 1
struct group * brcm_getgrgid(gid_t gid)
{
	int fd;
	static struct group *group;
	FILE *fp;
	
//	DEBUG(0, ("brcm_getgrgid: entered gid=%d\n",gid));
	/*	
	if (!ug_lock(GROUP_FILENAME, O_RDONLY, &fd))
	{
		return NULL;
	}
	
	if ((fp = fdopen(fd, "r")) == NULL)
	{
		DEBUG(1, ("vkp_getgrgid:1: open failed, errno=%u, str='%s'\n", errno, strerror(errno)));
		close(fd);
		return NULL;	
	}
        */

	if ((fd = open(GROUP_FILENAME, O_RDONLY)) < 0)
		return NULL;

      	while ((group = (struct group *) brcm_getgrent(fd)) != NULL)
		if (group->gr_gid == gid) {
//		        DEBUG(0, ("brcm_getgrgid: found group with gid=%d\n",gid));
			close(fd);
			return group;
		}

	close(fd);
	return NULL;
}
#endif

#if 1
struct group * brcm_getgrnam(const char *name)
{
	static struct group *group;
	FILE *fp;
	int fd;

	if (name == NULL || *name == '\0')
		return NULL;
	
//DEBUG(0, ("brcm_getgrnam enterred name=%s\n", name));
/*	
	if (!ug_lock(GROUP_FILENAME, O_RDONLY, &fd))
	{
		return NULL;
	}
	
	if ((fp = fdopen(fd, "r")) == NULL)
	{
		DEBUG(1, ("vkp_getgrgnam:1: open failed, errno=%u, str='%s'\n", errno, strerror(errno)));
		close(fd);
		return NULL;	
	}       
*/

	if ((fd = open(GROUP_FILENAME, O_RDONLY)) < 0)
		return NULL;

      	while ((group = (struct group *) brcm_getgrent(fd)) != NULL)
		if (!strcmp(group->gr_name,name)) {
//		  DEBUG(0, ("brcm_getgrnam found group with name=%s\n", name));
			close(fd);
			return group;
		}

	close(fd);
	return NULL;
}
#endif

#if 0
struct passwd * brcm_getpwuid(uid_t uid)
{
	static struct passwd result;
	static char line[256];
	FILE *fp;
	char *tok;
	
DEBUG(1, ("gew_getpwuid:1: entered uid=%u\n",uid));
	
	if ((fp = fopen(PASSWD_FILENAME, "r")) == NULL)
	{
DEBUG(1, ("gew_getpwuid:1: open failed, errno=%u, str='%s'\n", errno, strerror(errno)));
		return NULL;	
	}
	
	while (fgets(line, 256, fp) != NULL)
	{
		char *ptr = line;
DEBUG(1, ("gew_getpwuid:1: line='%s'\n", line));
	
		tok = strsep( &ptr, ":" );
		if (tok == NULL)
			return NULL;
			
		/* Use 'result' as a temporary. */
		result.pw_name		= tok;
		result.pw_passwd	= strsep( &ptr, ":" );
		result.pw_uid		= strtol( strsep( &ptr, ":" ), NULL, 10 );
		result.pw_gid		= strtol( strsep( &ptr, ":" ), NULL, 10 );
		result.pw_gecos		= strsep( &ptr, ":" );
		result.pw_dir		= strsep( &ptr, ":" );
		result.pw_shell		= strsep( &ptr, ":" );

		if (uid == result.pw_uid)
		{
DEBUG(1, ("gew_getpwuid:1: match! %s:%s:%u:%u:%s:%s:%s\n", result.pw_name,result.pw_passwd,result.pw_uid,result.pw_gid,result.pw_gecos,result.pw_dir,result.pw_shell));
			
			fclose( fp );
			return &result;
		}
	}
	fclose( fp );
	return NULL;	
}
#endif


/*
 * Define GR_SCALE_DYNAMIC if you want grp to dynamically scale its read buffer
 * so that lines of any length can be used.  On very very small systems,
 * you may want to leave this undefined becasue it will make the grp functions
 * somewhat larger (because of the inclusion of malloc and the code necessary).
 * On larger systems, you will want to define this, because grp will _not_
 * deal with long lines gracefully (they will be skipped).
 */
#undef GR_SCALE_DYNAMIC

#ifndef GR_SCALE_DYNAMIC
/*
 * If scaling is not dynamic, the buffers will be statically allocated, and
 * maximums must be chosen.  GR_MAX_LINE_LEN is the maximum number of
 * characters per line in the group file.  GR_MAX_MEMBERS is the maximum
 * number of members of any given group.
 */
#define GR_MAX_LINE_LEN 128
/* GR_MAX_MEMBERS = (GR_MAX_LINE_LEN-(24+3+6))/9 */
#define GR_MAX_MEMBERS 11

#endif /* !GR_SCALE_DYNAMIC */

/*
 * This is the core group-file read function.  It behaves exactly like
 * getgrent() except that it is passed a file descriptor.  getgrent()
 * is just a wrapper for this function.
 */
struct group *brcm_getgrent(int grp_fd)
{
#ifndef GR_SCALE_DYNAMIC
	static char line_buff[GR_MAX_LINE_LEN];
	static char *members[GR_MAX_MEMBERS];
#else
	static char *line_buff = NULL;
	static char **members = NULL;
	short line_index;
	short buff_size;
#endif
	static struct group group;
	register char *ptr;
	char *field_begin;
	short member_num;
	char *endptr;
	int line_len;


	/* We use the restart label to handle malformatted lines */
  restart:
#ifdef GR_SCALE_DYNAMIC
	line_index = 0;
	buff_size = 256;
#endif

#ifndef GR_SCALE_DYNAMIC
	/* Read the line into the static buffer */
	if ((line_len = read(grp_fd, line_buff, GR_MAX_LINE_LEN)) <= 0)
		return NULL;
	field_begin = strchr(line_buff, '\n');
	if (field_begin != NULL)
		lseek(grp_fd, (long) (1 + field_begin - (line_buff + line_len)),
			  SEEK_CUR);
	else {						/* The line is too long - skip it :-\ */

		do {
			if ((line_len = read(grp_fd, line_buff, GR_MAX_LINE_LEN)) <= 0)
				return NULL;
		} while (!(field_begin = strchr(line_buff, '\n')));
		lseek(grp_fd, (long) ((field_begin - line_buff) - line_len + 1),
			  SEEK_CUR);
		goto restart;
	}
	if (*line_buff == '#' || *line_buff == ' ' || *line_buff == '\n' ||
		*line_buff == '\t')
		goto restart;
	*field_begin = '\0';

#else							/* !GR_SCALE_DYNAMIC */
	line_buff = realloc(line_buff, buff_size);
	while (1) {
		if ((line_len = read(grp_fd, line_buff + line_index,
							 buff_size - line_index)) <= 0)
			return NULL;
		field_begin = strchr(line_buff, '\n');
		if (field_begin != NULL) {
			lseek(grp_fd,
				  (long) (1 + field_begin -
						  (line_len + line_index + line_buff)), SEEK_CUR);
			*field_begin = '\0';
			if (*line_buff == '#' || *line_buff == ' '
				|| *line_buff == '\n' || *line_buff == '\t')
				goto restart;
			break;
		} else {				/* Allocate some more space */

			line_index = buff_size;
			buff_size += 256;
			line_buff = realloc(line_buff, buff_size);
		}
	}
#endif							/* GR_SCALE_DYNAMIC */

	/* Now parse the line */
	group.gr_name = line_buff;
	ptr = strchr(line_buff, ':');
	if (ptr == NULL)
		goto restart;
	*ptr++ = '\0';

	group.gr_passwd = ptr;
	ptr = strchr(ptr, ':');
	if (ptr == NULL)
		goto restart;
	*ptr++ = '\0';

	field_begin = ptr;
	ptr = strchr(ptr, ':');
	if (ptr == NULL)
		goto restart;
	*ptr++ = '\0';

	group.gr_gid = (gid_t) strtoul(field_begin, &endptr, 10);
	if (*endptr != '\0')
		goto restart;

	member_num = 0;
	field_begin = ptr;

#ifndef GR_SCALE_DYNAMIC
	while ((ptr = strchr(ptr, ',')) != NULL) {
		*ptr = '\0';
		ptr++;
		members[member_num] = field_begin;
		field_begin = ptr;
		member_num++;
	}
	if (*field_begin == '\0')
		members[member_num] = NULL;
	else {
		members[member_num] = field_begin;
		members[member_num + 1] = NULL;
	}
#else							/* !GR_SCALE_DYNAMIC */
	free(members);
	members = (char **) malloc((member_num + 1) * sizeof(char *));   
	for ( ; field_begin && *field_begin != '\0'; field_begin = ptr) {
	    if ((ptr = strchr(field_begin, ',')) != NULL)
		*ptr++ = '\0';
	    members[member_num++] = field_begin;
	    members = (char **) realloc(members,
		    (member_num + 1) * sizeof(char *));
	}
	members[member_num] = NULL;
#endif							/* GR_SCALE_DYNAMIC */

	group.gr_mem = members;
	return &group;
}


#define PWD_BUFFER_SIZE 256

/* This isn't as flash as my previous version -- it doesn't dynamically
  scale down the gecos on too-long lines, but it also makes fewer syscalls,
  so it's probably nicer.  Write me if you want the old version.  Maybe I
  should include it as a build-time option... ?
  -Nat <ndf@linux.mit.edu> */

struct passwd *brcm__getpwent(int pwd_fd)
{
	static char line_buff[PWD_BUFFER_SIZE];
	static struct passwd passwd;
	char *field_begin;
	char *endptr;
	char *gid_ptr=NULL;
	char *uid_ptr=NULL;
	int line_len;
	int i;

	/* We use the restart label to handle malformatted lines */
  restart:
	/* Read the passwd line into the static buffer using a minimal of
	   syscalls. */
	if ((line_len = read(pwd_fd, line_buff, PWD_BUFFER_SIZE)) <= 0)
		return NULL;
	field_begin = strchr(line_buff, '\n');
	if (field_begin != NULL)
		lseek(pwd_fd, (long) (1 + field_begin - (line_buff + line_len)),
			  SEEK_CUR);
	else {						/* The line is too long - skip it. :-\ */

		do {
			if ((line_len = read(pwd_fd, line_buff, PWD_BUFFER_SIZE)) <= 0)
				return NULL;
		} while (!(field_begin = strchr(line_buff, '\n')));
		lseek(pwd_fd, (long) (field_begin - line_buff) - line_len + 1,
			  SEEK_CUR);
		goto restart;
	}
	if (*line_buff == '#' || *line_buff == ' ' || *line_buff == '\n' ||
		*line_buff == '\t')
		goto restart;
	*field_begin = '\0';

	/* We've read the line; now parse it. */
	field_begin = line_buff;
	for (i = 0; i < 7; i++) {
		switch (i) {
		case 0:
			passwd.pw_name = field_begin;
			break;
		case 1:
			passwd.pw_passwd = field_begin;
			break;
		case 2:
			uid_ptr = field_begin;
			break;
		case 3:
			gid_ptr = field_begin;
			break;
		case 4:
			passwd.pw_gecos = field_begin;
			break;
		case 5:
			passwd.pw_dir = field_begin;
			break;
		case 6:
			passwd.pw_shell = field_begin;
			break;
		}
		if (i < 6) {
			field_begin = strchr(field_begin, ':');
			if (field_begin == NULL)
				goto restart;
			*field_begin++ = '\0';
		}
	}
	passwd.pw_gid = (gid_t) strtoul(gid_ptr, &endptr, 10);
	if (*endptr != '\0')
		goto restart;

	passwd.pw_uid = (uid_t) strtoul(uid_ptr, &endptr, 10);
	if (*endptr != '\0')
		goto restart;

	return &passwd;
}

