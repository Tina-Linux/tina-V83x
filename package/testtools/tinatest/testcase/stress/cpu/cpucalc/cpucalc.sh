#!/bin/ash
echo "===$0 start!==="
echo "Default execut: stress-ng --all 5"

if [ $# = 0 -o x$1 = "x--help" ] ; then
	echo "you can execute : 'stress-ng --help' to view more parameters !"
	echo "here I've only done a few simple combination tests."
	echo "pi:Generating 5 worker for circumference algorithm to Generate pressure."
	echo "sock:Generate 5 worker calling socket correlation function to generate pressure."
	echo "sequential:To run 5 simultaneous instances of all the stressors sequentially one by one."
	echo "all:To run 5 instances of all the stressors for 10 minutes."
	exit
fi

cacl_test()
{
	case "$1" in
		pi)
			stress-ng --cpu 2 --cpu-method pi --timeout 150
			;;
		sock)
			stress-ng --sock 2 --timeout 150
			;;
		sequential)
			stress-ng --sequential 2 --timeout 150
			;;
		all)
			stress-ng --cpu 2 --cpu-method all --timeout 150
			;;
		*)
			echo "Without this combination."
			return
			;;
	esac

}
cacl_test $1

echo "===$0 success==="
