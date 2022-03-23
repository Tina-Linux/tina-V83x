#/bin/sh
tt_path="/stress/storage/throughput"
check_dir=`mjson_fetch ${tt_path}/check_directory`
percent=`mjson_fetch ${tt_path}/check_percent_of_free`
jobs=`mjson_fetch ${tt_path}/jobs`

[ -z "${percent}" ] && percent=95
! [ "${percent}" -gt 0 -a "${percent}" -lt 100 ] \
    && echo "Bad percent, value between 1-99" \
    && exit 1

mkdir -p ${check_dir} 2>/dev/null
! [ -d "${check_dir}" ] \
    && echo "Not Directory: ${check_dir}" \
    && exit 1

[ -z "${jobs}" ] && jobs=1
! [ "${jobs}" -gt 0 -a "${jobs}" -le 5 ] \
    && echo "Bad jobs, value between 1-5" \
    && exit 1

rwcheck -d ${check_dir} -b 128k -p ${percent} -t 10000000 -j ${jobs}
