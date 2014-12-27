#!/bin/sh
# script to find and print "TODO:" comments in files
# in a neat and pretty format

SRCDIRS="kernel userland"

for thing in $SRCDIRS; do
	find $thing -name "*.[ch]" |
	while read fname; do 
		fgrep -l "TODO:" "$fname" | sed 's/.*/==> &/'
		fgrep -n "TODO:" "$fname" |
		sed 's/\([0-9]*\).*TODO:/    \1 -/g' |
		sed 's/\*\///g'
	done
done
