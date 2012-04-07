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
 *
 * MTD utility functions
 *
 * Copyright 2006, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: mtd.h,v 1.2 2007/03/19 09:37:35 shinjung Exp $
 */

#ifndef _mtd_h_
#define _mtd_h_

/*
 * Open an MTD device
 * @param	mtd	path to or partition name of MTD device
 * @param	flags	open() flags
 * @return	return value of open()
 */
extern int mtd_open(const char *mtd, int flags);

/*
 * Erase an MTD device
 * @param	mtd	path to or partition name of MTD device
 * @return	0 on success and errno on failure
 */
extern int mtd_erase(const char *mtd);

/*
 * Write a file or a URL to an MTD device
 * @param	path	file to write or a URL
 * @param	mtd	path to or partition name of MTD device 
 * @return	0 on success and errno on failure
 */
extern int mtd_write(const char *path, const char *mtd);

#endif /* _mtd_h_ */
