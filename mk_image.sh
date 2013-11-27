#!/bin/sh
machine=$(uname)

if [ `id -u` -ne 0 ]; then
	echo "Need to run as root."
	sudo $0
	exit 0
fi

function copy_stuff(){
	mkdir -p temp_mount/boot
	cp kern/obsidian-i586.bin temp_mount/boot/
	cp initrd.img temp_mount/boot/
	echo "Copied" temp_mount/boot/*
}

if [ $machine = "FreeBSD" ]; then

	echo "Generating on freebsd..."
	mkdir temp_mount
	mdconfig -a -t vnode -f vdrive.hdd -u 1
	mount -t msdosfs /dev/msdosfs/NO_NAME temp_mount

	copy_stuff

	umount temp_mount
	mdconfig -d -u 1
	rmdir temp_mount
	echo "Done"

elif [ $machine = "Linux" ]; then

	echo "Generating on linux..." 
	mkdir temp_mount
	losetup -o$(( 63 * 512 )) /dev/loop0 vdrive.hdd
	mount -t vfat /dev/loop0 temp_mount

	copy_stuff

	umount temp_mount
	losetup -d /dev/loop0
	rmdir temp_mount
	echo "Done"
fi
