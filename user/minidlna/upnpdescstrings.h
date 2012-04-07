/* miniupnp project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006 Thomas Bernard
 * This software is subject to the coditions detailed in
 * the LICENCE file provided within the distribution */
#ifndef __UPNPDESCSTRINGS_H__
#define __UPNPDESCSTRINGS_H__

#include "config.h"

/* strings used in the root device xml description */
#ifdef NETGEAR
 #define ROOTDEV_MANUFACTURERURL	"http://www.netgear.com/"
 #define ROOTDEV_MANUFACTURER		"NETGEAR"
 #define ROOTDEV_MODELNAME		"Windows Media Connect compatible (ReadyDLNA)"
 #define ROOTDEV_MODELURL		OS_URL
 #ifdef READYNAS
  #define ROOTDEV_MODELDESCRIPTION	"ReadyDLNA on ReadyNAS RAIDiator OS"
 #else
  #define ROOTDEV_MODELDESCRIPTION	"ReadyDLNA"
 #endif
#else
 #define ROOTDEV_MANUFACTURERURL	"http://www.asus.com/"
 #define ROOTDEV_MANUFACTURER		"ASUSTeK"
 #define ROOTDEV_MODELNAME		"Windows Media Connect compatible"
 #define ROOTDEV_MODELDESCRIPTION	"RT-N56U"
 #define ROOTDEV_MODELURL		"http://www.asus.com/"
#endif

#endif
