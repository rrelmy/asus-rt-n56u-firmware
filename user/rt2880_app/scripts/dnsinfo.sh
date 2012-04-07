#! /bin/sh

echo ">"
echo "nvram *proto: >"
nvram show|grep proto

echo ">"
echo "nvram dns*: >"
nvram show|grep dns

echo ">"
echo "nvram link*: >"
nvram show|grep link

echo ">"
echo "nvram *ipaddr: >"
nvram show|grep ipaddr

echo ">"
echo "ifconfig: >"
ifconfig

echo ">"
echo "route: >"
route

echo ">"
echo "resolv: >"
cat /etc/resolv.conf

echo ">"
echo "udhcpd.conf: >"
cat /tmp/udhcpd.conf

echo ">"
echo "ping: >"
ping 8.8.8.8

echo ">"
echo "syslog: >"
cat /tmp/syslog.log

