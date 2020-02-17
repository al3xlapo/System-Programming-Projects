#!/bin/bash

#path

ACTIVE=
act=0
INACTIVE=
inact=0
path=$1
i=0

for dir in $path/*; do
	if [ -f "$dir/_pid" ]
		then ACTIVE+=("$dir")
		let act=act+1
	else
		INACTIVE+=("$dir")
		let inact=inact+1
	fi
done

if [ $act -eq 1 ]; then
	echo $act board active
else
	echo $act boards active
fi

while [ $i -le $act ]; do
	echo ${ACTIVE[i]}
	let i=i+1
done
let i=0

if [ $inact -eq 1 ]; then
	echo $inact inactive board
else
	echo $inact inactive boards
fi

while [ $i -le $inact ]; do
	echo ${INACTIVE[i]}
	let i=i+1
done

