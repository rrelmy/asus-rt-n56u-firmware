#!/usr/bin/tclsh

# Wrapper for usb_modeswitch, called by
# /etc/udev/rules.d/80-usb_modeswitch.rules
#
# Does ID check on hotplugged USB devices and calls the
# mode switching program with the matching parameter file
# from /etc/usb_modeswitch.d
#
# Version 1.0.7


# (C) Josua Dietze 2009, 2010
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details:
#
# http://www.gnu.org/licenses/gpl.txt



# Change this to 1 if you want a simple logging
# to /var/log/usb_modeswitch

set logging 0


# Execution starts at the file bottom

proc {Main} {argc argv} {

global scsi usb match wc logging
set dbdir	/etc/usb_modeswitch.d
set bindir	/usr/sbin

Log "raw args from udev: $argv"


# Mapping of the short string identifiers (in the config
# file names) to the long name used here
#
# If we need them it's a snap to add new attributes here!

set match(sVe) scsi(vendor)
set match(sMo) scsi(model)
set match(sRe) scsi(rev)
set match(uMa) usb(manufacturer)
set match(uPr) usb(product)
set match(uSe) usb(serial)

# argv contains the values provided from the udev rule
# separated by "/"

set argList [split [lindex $argv 0] /]

# The "ready-to-eat" values from the udev command
# Update: Ubuntu Karmic udev substitution is quirky.
# Read the values directly from sysfs

set usb(idVendor) ""
set usb(idProduct) ""
set usb(manufacturer) ""
set usb(product) ""
set usb(serial) ""

# We don't know these yet either

set scsi(vendor) ""
set scsi(model) ""
set scsi(rev) ""

# arg 0: the bus id for the device (udev: %b)
# arg 1: the "kernel name" for the device (udev: %k)
#
# Both together give the top directory where the path
# to the SCSI attributes can be determined (further down)
# Addendum: older kernel/udev version seem to differ in
# providing these attributes - or not. So more probing
# is needed

if {[string length [lindex $argList 0]] == 0} {
	if {[string length [lindex $argList 1]] == 0} {
		Log "No device number values given from udev! Exiting"
		SafeExit
	} else {
		Log "Bus ID for device not given by udev."
		Log " Trying to determine it from kernel name ([lindex $argList 1]) ..."
		if {![regexp {^(\d-\d):} [lindex $argList 1] d dev_top]} {
			Log "Could not determine top device dir from udev values! Exiting"
			SafeExit
		}
	}
} else {
	set dev_top [lindex $argList 0]
}

set devdir /sys/bus/usb/devices/$dev_top
if {![file isdirectory $devdir]} {
	Log "Top sysfs directory (bus ID) for device not found! Exiting"
	SafeExit
}

# Now reading the USB attributes

foreach attr {idVendor idProduct manufacturer product serial} {
	if [file exists $devdir/$attr] {
		set rc [open $devdir/$attr r]
		set usb($attr) [read -nonewline $rc]
		close $rc
	}
}

Log "----------------\nUSB values from sysfs:"
foreach attr {manufacturer product serial} {
	Log "  $attr\t$usb($attr)"
}
Log "----------------"


# Getting the SCSI values via libusb results in a detached
# usb-storage driver. Not good for devices that want to be
# left alone. Fortunately, the sysfs tree provides the values
# too without need for direct access

# First we wait until the SCSI data is ready - or timeout.
# Timeout means: no storage driver was bound to the device.
# We run 20 times max, every half second (max. 10 seconds
# total)

# We also check if the device itself changes, probably
# because it was switched by the kernel (or even unplugged).
# Then we do simply nothing and exit quietly ...

set counter 0
while {$counter < 20} {
	after 500
	incr counter

	set sysdir $devdir/[lindex $argList 1]

	if {![file isdirectory $sysdir]} {
		# Device is gone. Unplugged? Switched by kernel?
		Log "sysfs device tree is gone; exiting"
		SafeExit
	}
	set rc [open $devdir/product r]
	set newproduct [read -nonewline $rc]
	close $rc
	if {![string match $newproduct $usb(product)]} {
		# Device has just changed. Switched by someone else?
		Log "device has changed; exiting"
		SafeExit
	}

	# Searching the storage/SCSI tree; might take a while
	if {[set dirList [glob -nocomplain $sysdir/host*]] != ""} {
		set sysdir [lindex $dirList 0]
		if {[set dirList [glob -nocomplain $sysdir/target*]] != ""} {
			set sysdir [lindex $dirList 0]
			regexp {.*target(.*)} $sysdir d subdir
			if {[set dirList [glob -nocomplain $sysdir/$subdir*]] != ""} {
				set sysdir [lindex $dirList 0]
				if [file exists $sysdir/vendor] {
					# Finally SCSI structure is ready, get the values
					ReadStrings $sysdir
					Log "SCSI values read"
					break
				}
			}
		}
	}
}
if {$counter == 20 && [string length $scsi(vendor)] == 0} {
	Log "SCSI tree not found; you may want to check if this path/file exists:"
	Log "$sysdir/vendor\n"
} else {
	Log "----------------\nSCSI values from sysfs:"
	foreach attr {vendor model rev} {
		Log " $attr\t$scsi($attr)"
	}
	Log "----------------"
}

# If SCSI tree in sysfs was not identified, try and get the values
# from a (nonswitching) call of usb_modeswitch; this detaches the
# storage driver, so it's just the last resort

if {$scsi(vendor)==""} {
	set testSCSI [exec $bindir/usb_modeswitch -v 0x$usb(idVendor) -p 0x$usb(idProduct)]
	regexp {  Vendor String: (.*?)\n} $testSCSI d scsi(vendor)
	regexp {   Model String: (.*?)\n} $testSCSI d scsi(model)
	regexp {Revision String: (.*?)\n} $testSCSI d scsi(rev)
	Log "SCSI values from usb_modeswitch:"
	foreach attr {vendor model rev} {
		Log " $attr\t$scsi($attr)"
	}
}

# If we don't have the SCSI values by now, we just
# leave the variables empty; they won't match anything

# Time to check for a matching config file.
# Matching itself is done by MatchDevice
#
# Sorting the configuration file names reverse so that
# the ones with matching additions are tried first; the
# common configs without matching are used at the end and
# provide a kind of fallback

set report {}
set configList [glob -nocomplain $dbdir/$usb(idVendor):$usb(idProduct)*]
foreach configuration [lsort -decreasing $configList] {
	Log "checking config: $configuration"
	if [MatchDevice $configuration] {
		set switch_config $configuration
		set devList1 [glob -nocomplain /dev/ttyUSB* /dev/ttyACM* /dev/ttyHS*]
		Log "! matched, now switching"
		if $logging {
			Log " (running command: $bindir/usb_modeswitch -I -W -c $configuration)"
			set report [exec $bindir/usb_modeswitch -I -W -c $configuration 2>@ stdout]
		} else {
			set report [exec $bindir/usb_modeswitch -I -Q -c $configuration]
		}
		Log "\nverbose output of usb_modeswitch:"
		Log "--------------------------------"
		Log $report
		Log "--------------------------------"
		Log "(end of usb_modeswitch output)\n"
		break
	} else {
		Log "* no match, not switching with this config"
	}
}

# We're finished with switching; success checking
# was done by usb_modeswitch and logged via syslog.
#
# If switching was OK we now check for drivers by
# simply recounting serial devices under /dev

if [regexp {ok:} $report] {
	# some settle time in ms
	Log "Now checking for new serial devices (driver bound?) ..."
	after 500
	set devList2 [glob -nocomplain /dev/ttyUSB* /dev/ttyACM* /dev/ttyHS*]
	if {[llength $devList1] >= [llength $devList2]} {
		Log " no new serial devices found, check driver"
		if [regexp {ok:(\w{4}):(\w{4})} $report d vend prod] {
			set idfile /sys/bus/usb-serial/drivers/option1/new_id
			if {![file exists $idfile]} {
				Log "\nTrying to load the option driver"
				set loader /sbin/modprobe
				Log " loader is: $loader"
				if [file exists $loader] {
					set result [exec $loader -v option]
					if {[regexp {not found} $result]} {
						Log " option driver not present as module"
					}
				} else {
						Log " /sbin/modprobe not found"
				}
			}
			if [file exists $idfile] {
				Log "Trying to add ID to option driver"
				catch {exec /bin/logger -p syslog.notice "usb_modeswitch.tcl: adding device ID $vend:$prod to driver \"option\""}
				exec echo "$vend $prod" >$idfile
				after 600
				set devList2 [glob -nocomplain /dev/ttyUSB* /dev/ttyACM*]
				if {[llength $devList1] >= [llength $devList2]} {
					Log " still no new serial devices found"
				} else {
					Log " driver successfully bound"
				}
			}
		}
	} else {
		Log " no new serial devices found"
	}
}

Log "\nAll done, exiting\n"
SafeExit

}
# end of proc {Main}


proc {ReadStrings} {dir} {

global scsi
Log "SCSI dir exists: $dir"

foreach attr {vendor model rev} {
	if [file exists $dir/$attr] {
		set rc [open $dir/$attr r]
		set scsi($attr) [read -nonewline $rc]
		close $rc
	}
}

}
# end of proc {ReadStrings}


proc {MatchDevice} {config} {

global scsi usb match

set devinfo [file tail $config]
set infoList [split $devinfo :]
set stringList [lrange $infoList 2 end]
if {[llength $stringList] == 0} {return 1}

foreach teststring $stringList {
	if {$teststring == "?"} {return 0}
	set tokenList [split $teststring =]
	set id [lindex $tokenList 0]
	set matchstring [lindex $tokenList 1]
	regsub -all {_} $matchstring { } matchstring
	Log "matching $match($id)"
	Log "  match string: $matchstring"
	Log " device string: [set $match($id)]"
	if {![string match $matchstring* [set $match($id)]] } {
		return 0
	}
}
return 1

}
# end of proc {MatchDevice}


proc {Log} {msg} {

global wc logging
if {$logging == 0} {return}
if {![info exists wc]} {
	set wc [open /var/log/usb_modeswitch a+]
	puts $wc "\n\nUSB_ModeSwitch log from [clock format [clock seconds]]\n"
}
puts $wc $msg

}
# end of proc {Log}


proc {SafeExit} {} {
global wc
if [info exists wc] {
	catch {close $wc}
}
exit

}
# end of proc {SafeExit}


# The actual entry point
Main $argc $argv
