#!/bin/bash

for (( len = 64; len < 32800; len *= 2))
do
	echo -e -n "$len \t"
	for key in 256 512 1023
	do
		elen=$(stat -c %s "tests/encrypted${len}_${key}")
		k=$(bc -l <<< "${len}.0 / ${elen}.0")
		echo -e -n "$elen\t"
	done
	echo
done
