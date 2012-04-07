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
* eventpriv.h
*
* Abstraction of select call into "event-handling" to make programming
* easier.  This header includes "private" definitions which users
* of the event-handling code should not care about.
*
* Copyright (C) 2001 Roaring Penguin Software Inc.
*
* This program may be distributed according to the terms of the GNU
* General Public License, version 2 or (at your option) any later version.
*
* $Id: eventpriv.h,v 1.2 2002/09/30 19:45:00 dskoll Exp $
*
* LIC: GPL
*
***********************************************************************/

#ifndef INCLUDE_EVENTPRIV_H
#define INCLUDE_EVENTPRIV_H 1
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/* Handler structure */
typedef struct EventHandler_t {
    struct EventHandler_t *next; /* Link in list                           */
    int fd;			/* File descriptor for select              */
    unsigned int flags;		/* Select on read or write; enable timeout */
    struct timeval tmout;	/* Absolute time for timeout               */
    EventCallbackFunc fn;	/* Callback function                       */
    void *data;			/* Extra data to pass to callback          */
} EventHandler;

/* Selector structure */
typedef struct EventSelector_t {
    EventHandler *handlers;	/* Linked list of EventHandlers            */
    int nestLevel;		/* Event-handling nesting level            */
    int opsPending;		/* True if operations are pending          */
    int destroyPending;		/* If true, a destroy is pending           */
} EventSelector;

/* Private flags */
#define EVENT_FLAG_DELETED 256
#endif
