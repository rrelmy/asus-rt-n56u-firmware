#! /bin/sh

#asus_mfg=`nvram get asus_mfg`
#echo "asus_mfg $asus_mfg"
#if [ $asus_mfg != "0" ]; then
#exit 1
#fi

echo "argv $0 $1 $2"
# echo "argv $0 $1 $2" >> /tmp/auto01

if [ "$1" == "" ]; then
echo "parameter is none"
# echo "parameter is none" >> /tmp/auto01
exit 1
fi

existed=`cat /proc/partitions | grep $1 | wc -l`
echo "existed $existed"
# echo "existed $existed" >> /tmp/auto01
if [ $existed == "0" ]; then
exit 1
fi

mounted=`mount | grep $1 | wc -l`
echo "mounted $mounted"
# echo "mounted $mounted" >> /tmp/auto01

country_code=`nvram get wl_country_code`
echo "country_code $country_code"
# echo "country_code $country_code" >> /tmp/auto01

# mounted, assume we umount
if [ $mounted -ge 1 ]; then
echo "R/media/$2" 
echo "try to umount /media/$2"
# echo "try to umount /media/$2" >> /tmp/auto01 
if ! umount "/media/$2"; then
echo "umount /media/$2 fail"
# echo "umount /media/$2 fail" >> /tmp/auto01
exit 1
fi

if ! rmdir "/media/$2"; then
echo "rmdir /media/$2 fail!"
# echo "rmdir /media/$2 fail!" >> /tmp/auto01
exit 1
fi
# not mounted, lets mount under /media
else
if ! mkdir -p "/media/$2"; then
echo "mkdir /media/$2 fail"
# echo "mkdir /media/$2 fail" >> /tmp/auto01
exit 1
fi

dosfsck -V "/dev/$1" > "/tmp/dosfsck_result_$1" 2>&1
e2fsck -p "/dev/$1" > "/tmp/e2fsck_result_$1" 2>&1
chkntfs --verbose --showminors -a -f "/dev/$1" > "/tmp/chkntfs_result_$1" 2>&1

if [ $country_code = "TW" ]; then
#	echo "country_code TW case" >> /tmp/auto01
	mount "/dev/$1" "/media/$2" -o iocharset=utf8,codepage=950
elif [ $country_code = "CN" ]; then
#	echo "country_code CN case" >> /tmp/auto01
	mount "/dev/$1" "/media/$2" -o iocharset=utf8,codepage=936
else
#	echo "country_code XX case" >> /tmp/auto01
	mount "/dev/$1" "/media/$2" -o iocharset=utf8
fi
mounted=`mount | grep $1 | wc -l`
if [ $mounted == "0" ]; then
# echo "mount try 1 fail" >> /tmp/auto01
mount "/dev/$1" "/media/$2"					# ext2, ext3
mounted=`mount | grep $1 | wc -l`
if [ $mounted == "0" ]; then
# echo "mount try 2 fail" >> /tmp/auto01
#ntfs-3g "/dev/$1" "/media/$2" -o force
#ntfs-3g "/dev/$1" "/media/$2" -o force,umask=0,locale=utf8	# ntfs
mount -t ufsd -o iocharset=utf8 -o force "/dev/$1" "/media/$2"	# ntfs
# failed to mount, clean up mountpoint
mounted=`mount | grep $1 | wc -l`
if [ $mounted == "0" ]; then
# echo "mount try 3 fail" >> /tmp/auto01
if ! rmdir "/media/$2"; then
echo "rmdir /media/$2 fail!!"
# echo "rmdir /media/$2 fail!!" >> /tmp/auto01
exit 1
fi
fi
fi
fi
echo "A/media/$2" 
# echo "mounted /dev/$1 @ /media/$2" >> /tmp/auto01
fi

# rc need to know the event.
killall -SIGTTIN init
exit 0
