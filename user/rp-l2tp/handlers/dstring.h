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
/***********************************************************************
*
* l2tp/handlers/dstring.h
*
* Declares simple buffer which grows to accomodate accumulated string
* data
*
* Copyright (C) 2002 by Roaring Penguin Software Inc.
*
* LIC: GPL
*
***********************************************************************/

#include <stdlib.h>

typedef struct {
    size_t alloc_size;
    size_t actual_size;
    char *data;
} dynstring;

int dynstring_init(dynstring *str);
void dynstring_free(dynstring *str);
int dynstring_append(dynstring *str, char const *s2);
int dynstring_append_len(dynstring *str, char const *s2, size_t len);
char const *dynstring_data(dynstring *str);
