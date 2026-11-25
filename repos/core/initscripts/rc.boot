#!/bin/sh

PATH=/bin:/usr/bin:/sbin:/usr/sbin

[ -f /etc/logo ] && cat /etc/logo
echo "> Qwaderton Linux booting..."

echo "> Mounting virtual filesystems..."
mountpoint -q /proc    || mount -t proc proc /proc -o nosuid,noexec,nodev
mountpoint -q /sys     || mount -t sysfs sys /sys -o nosuid,noexec,nodev
mountpoint -q /run     || mount -t tmpfs run /run -o mode=0755,nosuid,nodev
mountpoint -q /dev     || mount -t devtmpfs dev /dev -o mode=0755,nosuid
mkdir -p /run/lock /dev/pts /dev/shm /run/runit
mountpoint -q /dev/pts || mount /dev/pts >/dev/null 2>&1 || mount -t devpts devpts /dev/pts -o mode=0620,gid=5,nosuid,noexec
mountpoint -q /dev/shm || mount /dev/shm >/dev/null 2>&1 || mount -t tmpfs shm /dev/shm -o mode=1777,nosuid,nodev

if [ "$(command -v udevd)" ]; then
	echo "> Starting udevd daemon..."
	udevd --daemon
	udevadm trigger --action=add    --type=subsystems
	udevadm trigger --action=add    --type=devices
	udevadm settle
else
	# In the case the user may preffer mdevd as opposed to busybox mdev
	if [ ! -d "/var/service/mdevd" ]; then
		echo "> Starting mdev..."
		echo "/sbin/mdev" > /proc/sys/kernel/hotplug
		mdev -s
	fi
	find /sys -name 'modalias' -type f -exec cat '{}' + | sort -u | xargs modprobe -b -a 2>/dev/null
	ln -s /proc/self/fd /dev/fd
	ln -s fd/0          /dev/stdin
	ln -s fd/1          /dev/stdout
	ln -s fd/2          /dev/stderr
fi

echo "> Loading kernel modules..."
if [ -f /etc/rc.modules ]; then
    while read -r module; do
        [ -n "$module" ] && ! echo "$module" | grep -q '^#' && modprobe "$module"
    done < /etc/rc.modules
fi

echo "> Bringing up loopback interface..."
ip link set lo up

echo "> Remounting root as read-only..."
mount -o remount,ro /

for arg in $(cat /proc/cmdline); do
    case $arg in
         fastboot) FASTBOOT=1;;
        forcefsck) FORCEFSCK="-f";;
    esac
done

if [ ! "$FASTBOOT" ]; then
	echo "> Running filesystem check..."
	fsck $FORCEFSCK -ATat noopts=_netdev
	if [ "$?" -gt 1 ]; then
		echo "*******************************************"
		echo "**        Filesystem check failed        **"
		echo "** You been dropped to maintenance shell **"
		echo "*******************************************"
		sulogin -p
	fi
fi

echo "> Enabling swap..."
swapon -a

echo "> Remounting root as read-write..."
mount -o remount,rw /

echo "> Mounting all filesystems..."
mount -a

if [ ! -f /etc/hostname ]; then
	echo localhost > /etc/hostname
fi
echo "> Setting hostname..."
hostname -F /etc/hostname

echo "> Cleaning stale PID files and /tmp..."
find /var/run -name "*.pid" -delete
find /tmp -xdev -mindepth 1 ! -name lost+found -delete

if [ -f "/etc/sysctl.conf" ]; then
	echo "> Applying sysctl settings..."
	sysctl -q -p
fi

if [ -e /dev/rtc ] || [ -e /dev/rtc0 ]; then
	echo "> Restoring hardware clock..."
	hwclock -s -u
fi

if [ -f "/var/lib/random-seed" ]; then
	echo "> Seeding /dev/urandom..."
	cat /var/lib/random-seed >/dev/urandom
	rm -f /var/lib/random-seed
fi

if [ -d /sys/firmware/efi/efivars ]; then
	echo "> Mounting efivarfs..."
	mount -t efivarfs none /sys/firmware/efi/efivars
fi

dmesg >/var/log/dmesg.log

if [ -x /etc/rc.boot.local ]; then
	echo "> Running local boot script..."
	/etc/rc.boot.local
fi

IFS=. read -r boottime _ < /proc/uptime
echo "booted in ${boottime}s..."
