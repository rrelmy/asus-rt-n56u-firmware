#! /bin/sh

#asus_mfg=`nvram get asus_mfg`
#echo "asus_mfg $asus_mfg"
#if [ $asus_mfg != "0" ]; then
#	exit 1
#fi

# echo "argv $0 $1 $2"
echo "argv $0 $1 $2" >> /tmp/auto01

if [ "$1" == "" ]; then
	echo "parameter is none"
	echo "parameter is none" >> /tmp/auto01
	exit 1
fi

existed=`cat /proc/partitions | grep $1 | wc -l`
echo "existed $existed"
echo "existed $existed" >> /tmp/auto01
if [ $existed == "0" ]; then
	exit 1
fi

mounted=`mount | grep $1 | wc -l`
echo "mounted $mounted"
echo "mounted $mounted" >> /tmp/auto01

country_code=`nvram get wl_country_code`
echo "country_code $country_code"
echo "country_code $country_code" >> /tmp/auto01

# mounted, assume we umount
if [ $mounted -ge 1 ]; then
	echo "try to umount /media/$2"
	echo "try to umount /media/$2" >> /tmp/auto01 
	if ! umount "/media/$2"; then
		echo "umount /media/$2 fail"
		echo "umount /media/$2 fail" >> /tmp/auto01
		exit 1
	fi

	if ! rmdir "/media/$2"; then
		echo "rmdir /media/$2 fail!"
		echo "rmdir /media/$2 fail!" >> /tmp/auto01
		exit 1
	fi
# not mounted, lets mount under /media
else
	if ! mkdir -p "/media/$2"; then
		echo "mkdir /media/$2 fail"
		echo "mkdir /media/$2 fail" >> /tmp/auto01
		exit 1
	fi

	asus_mfg=`nvram get asus_mfg`
	echo "asus_mfg $asus_mfg"
	echo "asus_mfg $asus_mfg" >> /tmp/auto01

	if [ $asus_mfg = "0" ]; then
		dosfsck -a -v "/dev/$1" > "/tmp/dosfsck_result_$1" 2>&1
		e2fsck -p -v "/dev/$1" > "/tmp/e2fsck_result_$1" 2>&1
		chkntfs -a -f --verbose "/dev/$1" > "/tmp/chkntfs_result_$1" 2>&1
	fi

	if [ $country_code = "TW" ]; then			# try vfat
		echo "vfat country_code TW case"
		echo "vfat country_code TW case" >> /tmp/auto01
		mount -t vfat "/dev/$1" "/media/$2" -o iocharset=utf8,codepage=950,shortname=winnt
	elif [ $country_code = "CN" ]; then
		echo "vfat country_code CN case"
		echo "vfat country_code CN case" >> /tmp/auto01
		mount -t vfat "/dev/$1" "/media/$2" -o iocharset=utf8,codepage=936,shortname=winnt
	else
		echo "vfat country_code XX case"
		echo "vfat country_code XX case" >> /tmp/auto01
		mount -t vfat "/dev/$1" "/media/$2" -o iocharset=utf8,shortname=winnt
	fi

	mounted=`mount | grep $1 | wc -l`
	if [ $mounted == "0" ]; then				# try ntfs
		echo "mount try 1 fail"
		echo "mount try 1 fail" >> /tmp/auto01
#ntfs-3g "/dev/$1" "/media/$2" -o force
#ntfs-3g "/dev/$1" "/media/$2" -o force,umask=0,locale=utf8
#mount -t ufsd -o iocharset=utf8 -o force "/dev/$1" "/media/$2"
		if [ $country_code = "TW" ]; then
			echo "ntfs country_code TW case"
			echo "ntfs country_code TW case" >> /tmp/auto01
			mount -t ufsd -o iocharset=utf8,codepage=950 -o force "/dev/$1" "/media/$2"
		elif [ $country_code = "CN" ]; then
			echo "ntfs country_code CN case"
			echo "ntfs country_code CN case" >> /tmp/auto01
			mount -t ufsd -o iocharset=utf8,codepage=936 -o force "/dev/$1" "/media/$2"
		else
			echo "ntfs country_code XX case"
			echo "ntfs country_code XX case" >> /tmp/auto01
			mount -t ufsd -o iocharset=utf8 -o force "/dev/$1" "/media/$2"
		fi

		mounted=`mount | grep $1 | wc -l`
		if [ $mounted == "0" ]; then			# try ext2/ext3
			echo "mount try 2 fail"
			echo "mount try 2 fail" >> /tmp/auto01
			mount -o user_xattr "/dev/$1" "/media/$2"

			# failed to mount, clean up mountpoint
			mounted=`mount | grep $1 | wc -l`
			if [ $mounted == "0" ]; then
				echo "mount try 3 fail" >> /tmp/auto01
				if ! rmdir "/media/$2"; then
					echo "rmdir /media/$2 fail!!"
					echo "rmdir /media/$2 fail!!" >> /tmp/auto01
					exit 1
				fi
			fi
		fi
	fi
echo "mounted /dev/$1 @ /media/$2"
echo "mounted /dev/$1 @ /media/$2" >> /tmp/auto01
fi

# rc need to know the event.
#killall -SIGTTIN init
exit 0

