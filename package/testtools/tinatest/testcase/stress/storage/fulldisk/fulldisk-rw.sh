#/bin/sh
tt_path="/stress/storage/fulldisk"
check_dir=`mjson_fetch ${tt_path}/check_directory`
percent=`mjson_fetch ${tt_path}/check_percent_of_free`

[ -z "${percent}" ] && percent=95
! [ "${percent}" -gt 0 -a "${percent}" -lt 100 ] \
    && echo "Bad percent, value between 1-99" \
    && exit 1

mkdir -p ${check_dir} 2>/dev/null
! [ -d "${check_dir}" ] \
    && echo "Not Directory: ${check_dir}" \
    && exit 1

rwcheck -d ${check_dir} -b 128k -p ${percent} -t 10000000
