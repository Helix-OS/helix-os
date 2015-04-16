#!/bin/sh
machine=$(uname)

if [ `id -u` -ne 0 ]; then
	echo "[ ] Need to run as root, trying sudo..."
	sudo $0
	exit 0
fi

IMAGE=helix.img
EXISTS=1

if [ -e $IMAGE ]; then
	EXISTS=0;
else
	dd if=/dev/zero of=$IMAGE bs=1M count=32 > /dev/null
	chmod uga+rw $IMAGE
fi

mkdir temp_mount

if [ $machine = "Linux" ]; then
	echo "[ ] Generating on linux..." 

	if [ $EXISTS -eq 1 ]; then
		echo -e "n\np\n\n2048\n+16M\na\nn\np\n\n34816\n\nw\nq\n" | fdisk $IMAGE > /dev/null
	fi

	losetup -o$(( 2048 * 512 )) /dev/loop0 $IMAGE

	if [ $EXISTS -eq 1 ]; then
		mkfs.vfat /dev/loop0 > /dev/null
	fi

	mount -t vfat /dev/loop0 temp_mount

	mkdir -p temp_mount/boot/grub
	cp -r build/* temp_mount/boot
	echo "[ ] Copied" temp_mount/boot/*

	if [ $EXISTS -eq 1 ]; then
		losetup /dev/loop1 $IMAGE
		grub-install --root-directory=$PWD/temp_mount/boot --boot-directory=$PWD/temp_mount/boot \
		    --no-floppy --modules="normal part_msdos ext2 multiboot vbe vga video_cirrus" /dev/loop1

		losetup -d /dev/loop1
	fi

	umount temp_mount
	losetup -d /dev/loop0

	losetup -o$(( 34816 * 512 )) /dev/loop0 $IMAGE

	if [ $EXISTS -eq 1 ]; then
		mkfs.vfat /dev/loop0 >/dev/null
	fi
	mount -t vfat /dev/loop0 temp_mount

	cp -r userland/build/* temp_mount

	echo "[ ] Copied" temp_mount/*

	umount temp_mount
	losetup -d /dev/loop0

elif [ $machine = "FreeBSD" ]; then
	echo "[E] Sorry, image generation on FreeBSD isn't supported at the moment (but should be coming soon)."
	#echo "Generating on freebsd..."

	#mdconfig -a -t vnode -f $IMAGE -u 1
	#mount -t msdosfs /dev/msdosfs/NO_NAME temp_mount

	#copy_stuff

	#umount temp_mount
	#mdconfig -d -u 1
else
	echo "[E] Sorry, image generation on $machine isn't supported at the moment."
fi

rmdir temp_mount
