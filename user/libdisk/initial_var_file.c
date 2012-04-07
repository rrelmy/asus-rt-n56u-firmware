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
#include <nvram/bcmnvram.h>
#include "disk_io_tools.h"
#include "disk_share.h"

int main(int argc, char *argv[]) {
	char *mount_path;
	
	if (argc != 2) {
		fprintf(stderr, "usage: %s MOUNT_DIR\n", argv[0]);
		return -1;
	}
	
	// 1. get the mount_path of the mount_path
	if (get_mount_path(argv[1], &mount_path) == -1) {
		fprintf(stderr, "Can't get the mount_path of %s.\n", argv[1]);
		return -1;
	}
	
	initial_all_var_file_in_mount_path(mount_path);
	
	free(mount_path);
	
	return 0;
}
