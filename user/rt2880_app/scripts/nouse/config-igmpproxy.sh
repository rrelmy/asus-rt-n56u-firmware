#!/bin/sh
#
# $Id: config-igmpproxy.sh,v 1.7 2009-03-30 08:20:06 yy Exp $
#
# usage: config-igmpproxy.sh <wan_if_name> <lan_if_name>
#

. /sbin/global.sh

igmpproxy.sh $wan_if $lan_if ppp0
killall -q igmpproxy
igmpproxy

