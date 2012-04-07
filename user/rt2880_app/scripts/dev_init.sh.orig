#!/bin/sh

#. /sbin/config.sh
#. /sbin/global.sh

genDevNode()
{
#Linux2.6 uses udev instead of devfs, we have to create static dev node by myself.
#if [ "$CONFIG_RT3XXX_EHCI_OHCI" != "" -o "$CONFIG_DWC_OTG" != "" -a "$CONFIG_HOTPLUG" == "y" ]; then
	mounted=`mount | grep mdev | wc -l`
	if [ $mounted -eq 0 ]; then
	mount -t ramfs mdev /dev
	mkdir /dev/pts
	mount -t devpts devpts /dev/pts
	mdev -s
	
	mknod   /dev/video0	c	81	0
	mknod   /dev/spiS0	c	217	0
	mknod   /dev/i2cM0	c	218	0
	mknod   /dev/rdm0	c	254	0
	mknod   /dev/flash0	c	200	0
	mknod   /dev/swnat0	c	210	0
	mknod   /dev/hwnat0	c	220	0
	mknod   /dev/acl0	c	230	0
	mknod   /dev/ac0	c	240	0
	mknod   /dev/mtr0	c	250	0
	mknod   /dev/gpio	c	252	0
	mknod   /dev/nvram	c	228	0
	mknod   /dev/PCM	c	233	0
	mknod   /dev/I2S	c	234	0
#fi

	echo "# <device regex> <uid>:<gid> <octal permissions> [<@|$|*> <command>]" > /etc/mdev.conf
	echo "# The special characters have the meaning:" >> /etc/mdev.conf
	echo "# @ Run after creating the device." >> /etc/mdev.conf
	echo "# $ Run before removing the device." >> /etc/mdev.conf
	echo "# * Run both after creating and before removing the device." >> /etc/mdev.conf
	echo "sd[a-z][1-9] 0:0 0660 */sbin/automount.sh \$MDEV" >> /etc/mdev.conf
	echo "sd[a-z] 0:0 0660 */sbin/automount.sh \$MDEV" >> /etc/mdev.conf
#	echo "sr[0-9] 0:0 0660 @/sbin/asus_sr \$MDEV \$ACTION" >> /etc/mdev.conf
#	echo "ttyUSB[0-9] 0:0 0660 */sbin/asus_tty \$MDEV \$ACTION" >> /etc/mdev.conf
#	echo "ttyACM[0-9] 0:0 0660 */sbin/asus_tty \$MDEV \$ACTION" >> /etc/mdev.conf
#	echo "[1-2]-[1-2]:1.[0-9] 0:0 0660 */sbin/asus_usb_interface \$MDEV \$ACTION" >> /etc/mdev.conf
#	if [ "$CONFIG_USB_SERIAL" = "y" ] || [ "$CONFIG_USB_SERIAL" = "m" ]; then
#		echo "ttyUSB0 0:0 0660 @/sbin/autoconn3G.sh connect" >> /etc/mdev.conf
#	fi
#	if [ "$CONFIG_BLK_DEV_SR" = "y" ] || [ "$CONFIG_BLK_DEV_SR" = "m" ]; then
#		echo "sr0 0:0 0660 @/sbin/autoconn3G.sh connect" >> /etc/mdev.conf
#	fi
#	if [ "$CONFIG_USB_SERIAL_HSO" = "y" ] || [ "$CONFIG_USB_SERIAL_HSO" = "m" ]; then
#		echo "ttyHS0 0:0 0660 @/sbin/autoconn3G.sh connect" >> /etc/mdev.conf
#	fi

	#enable usb hot-plug feature
	echo "/sbin/mdev" > /proc/sys/kernel/hotplug

fi
}

genDevNode
