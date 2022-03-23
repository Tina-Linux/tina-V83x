#/bin/sh
tt_base="/base/storage/rwdev"
devfile=`mjson_fetch ${tt_base}/devfile`
hexdata=`mjson_fetch ${tt_base}/hexdata`
do_write=`mjson_fetch ${tt_base}/do_write`
do_read=`mjson_fetch ${tt_base}/do_read`
do_verify=`mjson_fetch ${tt_base}/do_verify`
loop=`mjson_fetch ${tt_base}/loop_times`
buffer_size=`mjson_fetch ${tt_base}/buffer_size`
test_size=`mjson_fetch ${tt_base}/test_size`

[ -z "${devfile}" ] && echo "No device file, quit!!" && exit 1

com="rwdev"
# data
for arg in ${hexdata}
do
    com="${com} -d ${arg}"
done
# buffer_size
for arg in ${buffer_size}
do
    com="${com} -b ${arg}"
done
# test_size
[ -n "${test_size}" ] && com="${com} -s ${test_size}"
# action
[ "${do_write}" = "true" ] && com="${com} -a 0"
[ "${do_read}" = "true" ] && com="${com} -a 1"
[ "${do_verify}" = "true" ] && com="${com} -a 2"
# loop
[ -n "${loop}" ] && com="${com} -l ${loop}"
# devfile
com="${com} ${devfile}"

echo -e "\tCOMMAND: ${com}"
${com}
