#!/bin/sh

######################################################################
# file monitor-selfboot.sh
# version 2.0.0
# date 2017-10-11
# author liumingyuan <liumingyuan@allwinnertech.com>
# Copyright (c) 2017 Allwinner Technology. All Rights Reserved.
######################################################################

index=0
storage(){
	local a
	local lc
	local n
	local count
	local tmp
	if [ -f /tmp/processlog ];then
		lc=`cat /tmp/processlog|wc -l`
	else
		return 1
	fi
	a=1
	[ $lc -ne 0 ] && until [ $a -gt $lc ]
	do
		n=$a
		tp=$(awk 'NR=='$n' {print $1}' /tmp/processlog)
		count=`ps -w|grep "$tp"|grep -v grep|wc -l`
		if [ $count -ne 0 ]; then
			tmp=$(awk 'NR=='$n' {print}' /tmp/processlog)
			echo "$tmp" >> /tmp/monitorprocess
			eval ptm$index=\"$tmp\"
			let index++
		fi
		let a++
	done
}
print(){
	local g=0
	until [ $g -eq $index ]
	do
		eval echo \$ptm$index
		let g++
	done
}
monitor(){
	local x=0
	local T
	[ $index -ne 0 ] && until [ $x -eq $index ]
	do
		print
		eval T="\$ptm$x"
		count=`ps -w|grep "$T"|grep -v grep|wc -l`
		if [ $count -eq 0 ]; then
			eval exec $T &
		fi
		let x++
	done
}
while true
do
	sleep 10s
	if [ -f /tmp/processlog ];then
		break
	fi
done
storage
[ $? -eq 0 ] && while true
do
	monitor
	sleep 1s
done >/dev/null
