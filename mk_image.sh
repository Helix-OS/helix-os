#!/bin/sh
machine=$(uname)

if [ `id -u` -ne 0 ]; then
	echo "Need to run as root."
	sudo $0
	exit 0
fi

IMAGE=helix.img
dd if=/dev/zero of=$IMAGE bs=1M count=16
chmod uga+rw $IMAGE

function copy_stuff(){
	mkdir -p temp_mount/boot/grub

	cp kernel/helix_kernel-i586 temp_mount/boot/
	cp initrd.img temp_mount/boot/
	cp -r boot-image/* temp_mount/boot/

	echo "Copied" temp_mount/boot/*
}

mkdir temp_mount

if [ $machine = "Linux" ]; then
	echo "Generating on linux..." 

	echo -e "n\np\n\n2048\n\na\nw\nq\n" | fdisk $IMAGE
	losetup -o$(( 2048 * 512 )) /dev/loop0 $IMAGE
	mkfs.vfat /dev/loop0
	mount -t vfat /dev/loop0 temp_mount

	copy_stuff

	losetup /dev/loop1 $IMAGE
	grub-install --root-directory=$PWD/temp_mount/boot --boot-directory=$PWD/temp_mount/boot \
		--no-floppy --modules="normal part_msdos ext2 multiboot" /dev/loop1

	losetup -d /dev/loop1

	umount temp_mount
	losetup -d /dev/loop0

elif [ $machine = "FreeBSD" ]; then
	echo "Sorry, image generation on FreeBSD isn't supported at the moment (but should be coming soon)."
	#echo "Generating on freebsd..."

	#mdconfig -a -t vnode -f $IMAGE -u 1
	#mount -t msdosfs /dev/msdosfs/NO_NAME temp_mount

	#copy_stuff

	#umount temp_mount
	#mdconfig -d -u 1
else
	echo "Sorry, image generation on $machine isn't supported at the moment."
fi

rmdir temp_mount
echo "Done"
