#!/bin/sh
unset
version="0.8"

usage()
{
	printf "
	Usage: %s [-n iter] [-d delay] [-m] [-o FILE] [-h]
	-n NUM   Updates to show before exiting.
	-d NUM   Seconds to wait between update.
	-m unit: MB
	-v Display mtop version.
	-h Display this help screen.
	\n" $(basename $0)

	exit 1;
}

total=0
max=0
idx=0
tmax=0;

#Arg for program
count=0;
second=1;
unit="KB";

while getopts :n:d:mo:vh OPTION;
do
	case $OPTION in
		n)
			count=$OPTARG;
			;;
		d)
			second=$OPTARG;
			;;
		m)
			unit="MB";
			;;
		v)
			echo "version: $version"
			exit 0;
			;;
		h)
			usage
			;;
		?)
			usage
			;;
	esac
done
shift $((OPTIND - 1))

phs=$(ls /sys/class/hwmon/);

for p in $phs; do
	path="/sys/class/hwmon/$p";
	if [ -d ${path} ]; then
		if [ "$(cat $path/name)" == "mbus_pmu" ]; then
			break 5;
		fi
	fi
done

if [ ! -d ${path} ]; then
	path="/sys/class/hwmon/mbus_pmu/device";
	if [ ! -d ${path} ]; then
		printf "Path \"$path\" not exist. Please check it\n";
		exit 1;
	fi
fi

#Entry the work directory
#We do not need 'totddr' file, because some platform
#haven't it.
cd $path
if [ -e "$path/name" ]; then
	name=$(cat name)
	if [ "mbus_pmu" != $name ]; then
		printf "notice: name($name) != mbus_pmu \n"
		exit 1
	fi
fi

pmus=$(ls pmu_*);

pre_count="";
cur_count="";

transferunit()
{
	if [ $# -ne 1 ]; then
		echo "Bug: Can not transfer it";
		exit 1
	fi

	if [ $unit == "MB" ]; then
		echo $(awk "BEGIN{printf \"%.4f\", $1/1024}" 2>/dev/null)
	else
		echo $1
	fi
}

start_tm=$(awk '{printf $1}' /proc/uptime);

while [ $count -eq 0 ] || [ $idx -lt $count ]; do

	clear
	pre_count=${cur_count};
	cur_count="";

	if [ -n "${pre_count}" ]; then
		idx=$(expr ${idx} + 1);

		printf "\n----------------Num: %llu, Unit: %s, Interval: %ss----------------\n" \
			${idx} ${unit} ${second}

		printf "Total: %s,  " $(transferunit ${total})
		printf "Max: %s %s/s,  " ${max}  ${unit}

		if [ ${idx} -gt 1 ]; then
			printf "Average: %s %s/s\n" $(transferunit $(awk "BEGIN{printf \"%.4f\", \
				${total}/$(awk "BEGIN{printf \"%.4f\",${end_tm} - ${start_tm}}")}" 2>/dev/null)) $unit
		else
			printf "Average: 0.000 %s/s\n" $unit
		fi
	elif [ ${idx} -eq 0 ];then
		printf "\n----------------Num: %llu, Unit: %s, Interval: %ss----------------\n" \
			$idx $unit $second

		printf "Total: %s,  " $(transferunit $total)
		printf "Max: %s %s/s,  " $max  $unit
		printf "Average: 0.000 %s/s\n" $unit
	fi


	delta=0;
	pre=0;
	cur=0;
	tmax=0;

	pre_tm=${cur_tm};

	totddr="";

	for pmu in ${pmus};
	do
		cur=$(cat ${pmu} 2>/dev/null);
		# Save current count in the avriable
		cur_count="${cur_count}${cur_count:+ }${cur}"
	done
	cur_tm=$(awk '{printf $1}' /proc/uptime);

	if [ "${pre_count}"x != "x" ]; then
		cur_tmp=${cur_count};
		for pmu in ${pmus};
		do
			# shift the item for this pmu
			pre=${pre_count%% *};
			pre_count=${pre_count#* };

			cur=${cur_tmp%% *};
			cur_tmp=${cur_tmp#* };
			if [ "${pmu}" == "pmu_totddr" ]; then
				delta=$cur;
			else
				delta=$(awk "BEGIN{ \
						val=$pre; \
						if($cur < val) { \
							val = (0xFFFFFFFF - val); \
							val += $cur; \
						} else { \
							val = $cur - val; \
						}\
						printf(\"%u\", val); \
					}");
				tmax=$(awk "BEGIN{printf(\"%u\", ($tmax + $delta));}")
			fi


			if [ "${pmu}" == "pmu_totddr" ]; then
				totddr=$(printf "%s\t: %10s %s/s\n" ${pmu#*pmu_} $(transferunit \
					$(awk "BEGIN{printf \"%.4f\", $delta}" 2>/dev/null))  $unit)
			else
				printf "%s\t: %10s %s/s\n" ${pmu#*pmu_} $(transferunit \
					$(awk "BEGIN{printf \"%.4f\", $delta/$(awk "BEGIN{printf \"%.4f\",($cur_tm - $pre_tm)}")}" 2>/dev/null))  $unit
			fi
		done

		total=$(awk "BEGIN{printf(\"%d\", ($total + $tmax));}")
		tmax=$(transferunit $(awk "BEGIN{printf \"%.4f\", $tmax/($cur_tm - $pre_tm)}" 2>/dev/null));
		max=$(awk "BEGIN{if ($tmax > $max) {printf $tmax} else {printf $max}}");

		printf "--------\n"
		printf "SUM\t: %10s %s/s\n"  $tmax $unit
		echo "$totddr"
	fi

	sleep $second;
	end_tm=$(awk '{printf $1}' /proc/uptime);
done

exit 0;

