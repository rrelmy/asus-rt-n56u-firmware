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
/* This file is included into rc/rc.c */
/* donot add code unless required */


#ifndef _RCS5_H_
#define _RCS5_H_
	struct nvram_tuple robo[] = {
		{ "lan_ifname", "br0", 0 },
		{ "lan_ifnames", "eth0 eth1", 0 },
		{ "wan_ifname", "eth0", 0 },
		{ "wan_ifnames", "eth0", 0 },
		{ 0, 0, 0 }
	};

extern void rc1_start();
extern void rc1_stop();
extern void rc7_start();
extern void rc7_stop();
extern extra_restore_defaults();

#define RC1_START() 	rc1_start();

#define RC1_STOP() 	rc1_stop();

#define RC7_START() 	rc7_start();
#define RC7_STOP()	rc7_stop();

#define LINUX_OVERRIDES() if (bcm_is_robo() ) \
			linux_overrides = robo;						

#define EXTRA_RESTORE_DEFAULTS()	extra_restore_defaults()

#endif /* _RCS5_H_ */
