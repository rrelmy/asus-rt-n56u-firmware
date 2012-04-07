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
#ifndef CBCP_H
#define CBCP_H

typedef struct cbcp_state {
    int    us_unit;	/* Interface unit number */
    u_char us_id;		/* Current id */
    u_char us_allowed;
    int    us_type;
    char   *us_number;    /* Telefone Number */
} cbcp_state;

extern cbcp_state cbcp[];

extern struct protent cbcp_protent;

#define CBCP_MINLEN 4

#define CBCP_REQ    1
#define CBCP_RESP   2
#define CBCP_ACK    3

#define CB_CONF_NO     1
#define CB_CONF_USER   2
#define CB_CONF_ADMIN  3
#define CB_CONF_LIST   4
#endif
