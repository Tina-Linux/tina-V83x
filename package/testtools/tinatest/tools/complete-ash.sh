#!/bin/bash

get_top()
{
    local path
    path=`pwd`
    while [ ! "${path}" = "/" ]
    do
        [ -f "${path}/Makefile" ] \
            && grep "^PKG_NAME *:= *tinatest$" ${path}/Makefile >/dev/null \
            && break
        path=$(dirname ${path})
    done
    [ "${path}" = "/" ] && echo "Get top failed" && exit 1
    echo ${path}
}

# get_value <lower-config-path>
# example: /demo/demo-c/enable
get_value()
{
    local upper=$(sed '{s/[a-z]/\u&/g; s/[^ [:alnum:]]/_/g}' <<< ${1})
    eval "echo \${CONFIG_TINATEST${upper}}"
}

# check_config_existed <lower-config-path>
# example: /demo/demo-c/enable
check_config_existed()
{
    [ -n "$(get_value "$1")" ] && return 0 || return 1
}

get_valid_testcases()
{
    local one all
    all="$(find ${top}/testcase -name "private.conf" \
        | sed "s#^${top}/testcase\(.*\)/private.conf#\1#g")"
    for one in ${all}
    do
        check_config_existed "${one}/enable" && echo ${one}
    done
}

completion()
{
    local testcase dir
    for testcase in ${caselist}
    do
        dir="$(dirname ${testcase})"
        mkdir -p ${out}${dir}
        ! touch ${out}${testcase} \
            && echo "make completion failed: ${out}${testcase}" \
            && return 1
    done
    return 0
}

main()
{
    # check usage
    [ "$#" -ne 2 ] && echo "I Need Two Argumemts" && exit 1
    [ ! -f "$1" ] && echo "Not Found $1" && exit 1
    [ ! -d "$2" ] && echo "Not Found $2" && exit 1
    eval $(grep -vi '="default"' $1)

    out="$2"
    top="$(get_top)"
    caselist="$(get_valid_testcases)"
    completion
}

main $@
