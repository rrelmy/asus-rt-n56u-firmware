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
#ifndef _DPRINTF_H
#define _DPRINTF_H

extern void show_message(char *fmt, ...);

extern int options;
#ifndef OPT_DEBUG
#  define OPT_DEBUG       0x0001
#endif

#ifdef DEBUG
#  define dprintf2(f, fmt, ...) if( options & OPT_DEBUG ) \
{ \
  show_message("%s,%d: " fmt, __FILE__, __LINE__ , ##__VA_ARGS__); \
}
#  define dprintf(x)		dprintf2 x
#else
#  define dprintf(x)
#endif

#endif
