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
* event-tcp.h
*
* Event-driven TCP functions to allow for single-threaded "concurrent"
* server.
*
* Copyright (C) 2001 Roaring Penguin Software Inc.
*
* $Id: event_tcp.h,v 1.2 2002/09/30 19:45:00 dskoll Exp $
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 2 or (at your option) any later version.
*
* LIC: GPL
*
***********************************************************************/

#ifndef INCLUDE_EVENT_TCP_H
#define INCLUDE_EVENT_TCP_H 1

#include "event.h"
#include <sys/socket.h>

typedef void (*EventTcpAcceptFunc)(EventSelector *es,
				   int fd);

typedef void (*EventTcpConnectFunc)(EventSelector *es,
				    int fd,
				    int flag,
				    void *data);

typedef void (*EventTcpIOFinishedFunc)(EventSelector *es,
				       int fd,
				       char *buf,
				       int len,
				       int flag,
				       void *data);

#define EVENT_TCP_FLAG_COMPLETE 0
#define EVENT_TCP_FLAG_IOERROR  1
#define EVENT_TCP_FLAG_EOF      2
#define EVENT_TCP_FLAG_TIMEOUT  3

typedef struct EventTcpState_t {
    int socket;
    char *buf;
    char *cur;
    int len;
    int delim;
    EventTcpIOFinishedFunc f;
    EventSelector *es;
    EventHandler *eh;
    void *data;
} EventTcpState;

extern EventHandler *EventTcp_CreateAcceptor(EventSelector *es,
					     int socket,
					     EventTcpAcceptFunc f);

extern void EventTcp_Connect(EventSelector *es,
			     int fd,
			     struct sockaddr const *addr,
			     socklen_t addrlen,
			     EventTcpConnectFunc f,
			     int timeout,
			     void *data);

extern EventTcpState *EventTcp_ReadBuf(EventSelector *es,
				       int socket,
				       int len,
				       int delim,
				       EventTcpIOFinishedFunc f,
				       int timeout,
				       void *data);

extern EventTcpState *EventTcp_WriteBuf(EventSelector *es,
					int socket,
					char *buf,
					int len,
					EventTcpIOFinishedFunc f,
					int timeout,
					void *data);

extern void EventTcp_CancelPending(EventTcpState *s);

#endif
