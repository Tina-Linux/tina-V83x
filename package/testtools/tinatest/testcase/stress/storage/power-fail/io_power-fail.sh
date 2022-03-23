#!/bin/sh

################################################3
# Do read/write check for power failure.
# This testcase to check whether the data is correctness when
# device in power failure suddenly.
#
# options:
#   run_times: times for power failure.
#   check_directory: the directory to save data.
################################################3

check_dir=`mjson_fetch /stress/storage/power-fail/check_directory`
sec=`mjson_fetch /stress/storage/power-fail/sleep_before_test`

mkdir -p ${check_dir} 2>/dev/null
! [ -d "${check_dir}" ] \
    && echo "Not Directory: ${check_dir}" \
    && exit 1

serial="$(grep -o "console=[^ ]*" /proc/cmdline | sed 's/.*=\(.*\),.*/\1/')"
sleep ${sec}

if [ -z "${serial}" ]; then
    rwcheck -d ${check_dir} -b 128k -t 10000000
else
    rwcheck -d ${check_dir} -b 128k -t 10000000 &>/dev/${serial}
fi
