#!/bin/sh

deps=( 	make
	sh
	cc
	nasm
	grub-install
)

echo "Checking for needed dependancies..."

for dep in ${deps[@]}; do
	echo -n "Checking for $dep... "
	if ! which $dep; then
		echo "Need dependancy \"$dep\"."
		exit 1
	fi
done

echo "Have all needed build dependancies."
