#!/bin/sh

deps=( 	make
	sh
	cc
	nasm
	fdisk
	grub-install
	mkfs.vfat
)

echo "[ ] Checking for needed dependancies..."

for dep in ${deps[@]}; do
	echo -n "[ ] Checking for $dep... "
	if ! which $dep; then
		echo "[E] Need dependancy \"$dep\"."
		exit 1
	fi
done

echo "[ ] Have all needed build dependancies."
