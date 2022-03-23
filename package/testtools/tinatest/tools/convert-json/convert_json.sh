#!/bin/bash

#draw_indent <tab_cnt>
draw_indent() {
    local cnt
    for cnt in `seq $1`
    do
        echo -n "    "
    done
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

get_valid_system_config()
{
    local config_list mod_list
    mod_list=(
        ${top}/tools/convert-json/collectd.ls
        ${top}/tools/convert-json/outlog.ls
        ${top}/tools/convert-json/global.ls
        ${top}/tools/convert-json/info.ls
        ${top}/tools/convert-json/limit.ls
    )

    local module base config type
    for module in ${mod_list[@]}
    do
        base=$(grep "# *BASE *=.*$" ${module} \
            | awk -F "[= ]" '{print $NF}')

        for config in $(grep -v "^#" ${module})
        do
            type=$(get_type ${config})
            config=${config%:*}
            check_config_existed "${base}/${config}" \
                && echo ${base}/${config}:${type}
        done
    done
}

get_valid_testcases_config()
{
    local gconf_list=$(grep -hv "^#" \
        ${top}/tools/convert-json/task.ls \
        ${top}/tools/convert-json/info.ls \
        ${top}/tools/convert-json/limit.ls
    )

    for testcase in $(get_valid_testcases)
    do
        # pconf: path of private.conf
        local pconf="${top}/testcase${testcase}/private.conf"
        [ ! -f "${pconf}" ] && echo "No Found ${pconf}" && continue
        local pconf_list=$(egrep "^.*:.*=.*$" ${pconf} | awk -F ' ' '{print $1}')
        local config_list="${gconf_list} ${pconf_list}"

        # check all configs
        local config type
        for config in ${config_list}
        do
            type=$(get_type ${config})
            config=$(get_config ${config})
            check_config_existed "${testcase}/${config}" \
                && echo "${testcase}/${config}:${type}"
        done
    done
}

get_treelist()
{
    get_valid_system_config
    get_valid_testcases_config
}

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

# get_next <config-path:config-type>
# note:
#   get word follow @
# example:
#   /demo@demo-c/enable:bool
# @: means recursive call working at
# out:
#   demo-c
get_next()
{
    sed 's#.*@\([^/:]*\).*#\1#' <<< $1
}

# get_next <config-path:config-type>
# note:
#   get word before @
# example:
#   /class@demo-c/enable:bool
# @: means recursive call working at
# out:
#   class
get_pre()
{
    sed 's#.*/\([^/:]*\)@.*#\1#' <<< $1
}

# get_type <config-path:config-type>
# example:
#   /class/demo-c/enable:bool
# out:
#   bool
get_type()
{
    echo ${1##*:}
}

# get_config <config-path:config-type>
# example:
#   /class/demo-c/enable:bool
# out:
#   /class/demo-c/enable
get_config()
{
    echo ${1%:*}
}

# get_value <lower-config-path>
# example: /demo/demo-c/enable
get_value()
{
    local upper=$(sed '{s/[a-z]/\u&/g; s/[^ [:alnum:]]/_/g}' <<< ${1})
    eval "echo \${CONFIG_TINATEST${upper}}"
}

# check_end <config-path:config-type>
# note:
#   check whether the last part of config
# example:
#   @demo/demo-c/enable:bool => return 1
#   /demo@demo-c/enable:bool => return 1
#   /demo/demo-c@enable:bool => return 0
# @: means recursive call working at
check_end()
{
    egrep "@[^/]+:" &>/dev/null <<< $1 \
        && return 0 \
        || return 1
}

# move_next <config-path:config-type>
# note:
#   move @ to next /, call it must with check_end
# example:
#   /demo@demo-c/enable:bool
# @: means recursive call working at
# out:
#   /demo/demo-c@enable:bool
move_next()
{
    check_end $1 && return 1
    sed 's#\(.*\)@\([^/]*\)/\(.*\)#\1/\2@\3#' <<< $1
}

# convert_items <flag> <depth> <config1-path:config1-type> <config2-path:config2-type> ...
# flag decide whether draw comma in the last item
#   c: there are items in clist, so draw comma
#   e: there are no items in clist, so don't draw comma
convert_items()
{
    local flag=$1 && shift
    local depth=$1 && shift

    local branch type cnt val conf
    for branch in $@
    do
        cnt=$(( ${cnt} + 1 ))
        val=$(get_value $(sed 's#@#/#g; s/:.*//g' <<< ${branch}))
        conf=$(get_next ${branch})
        type=$(get_type ${branch})

        draw_indent ${depth} && echo -n "\"${conf}\" : "
        case "${type}" in
            "bool")
                [ "${val}" = "y" ] && echo -n "true" || echo -n "false"
                ;;
            "int")
                echo -n "${val}"
                ;;
            "array")
                local tmp num
                echo -n "["
                tmp=(${val})
                for num in $(seq 0 $(( ${#tmp[@]} - 1 )))
                do
                    echo -n " \"${tmp[${num}]}\""
                    [ "${num}" -ne $(( ${#tmp[@]} - 1 )) ] && echo -n ","
                done
                echo -n " ]"
                ;;
            "string")
                echo -n "\"$(sed 's#"#\\"#g' <<< ${val})\""
                ;;
        esac
        [ "${cnt}" -ge "$#" -a "${flag}" = "e" ] && echo || echo ","
    done
    local type=$(get_type)
}

# convert_json <config1-path:config1-type> <config2-path:config2-type> ...
# example:
#   /demo@demo-c/enable:bool
# @: means recursive call working at
convert_json() {
    local depth=$1 && shift
    [ "${depth}" -eq "0" ] && depth=1 && echo "\"/\" : {"

    # classify
    local trunks trunks_cnt trunk branch
    for branch in $@
    do
        trunk=$(get_next ${branch})
        ! grep -w "${trunk}" <<< ${trunks} &>/dev/null \
            && trunks="${trunks} ${trunk}" && trunks_cnt=$(( ${trunks_cnt} + 1 ))
        trunk="$(sed 's/[^ [:alnum:]]/_/g' <<< ${trunk})"
        eval "local ${trunk}=\"\${${trunk}} ${branch}\""
    done

    # draw json
    # elist means end list
    # clist means continue list,continue recursive call
    local elist clist cnt next
    for trunk in ${trunks}
    do
        draw_indent ${depth} && echo "\"${trunk}\" : {"

        trunk="$(sed 's/[^ [:alnum:]]/_/g' <<< ${trunk})"
        for branch in $(eval "echo \${${trunk}}")
        do
            next="$(move_next ${branch})"
            if check_end ${next}; then
                elist="${elist} ${next}"
            else
                clist="${clist} ${next}"
            fi
        done
        [ -n "${elist}" ] && convert_items \
            $([ -n "${clist}" ] && echo c || echo e) \
            $(( ${depth} + 1 )) ${elist}
        [ -n "${clist}" ] && convert_json $(( ${depth} + 1 )) ${clist}

        cnt=$(( ${cnt} + 1 ))
        if [ "${cnt}" -lt "${trunks_cnt}" ]; then
            draw_indent ${depth} && echo "},"
        else
            draw_indent ${depth} && echo "}"
        fi
        unset clist elist
    done

    [ "$(( ${depth} - 1 ))" -eq "0" ] && echo "}"
}

main()
{
    # check usage
    [ "$#" -ne 1 ] && echo "I Need Only One Argumemt" && exit 1
    [ ! -f "$1" ] && echo "Not Found $1" && exit 1
    eval $(grep -vi '="default"' $1)

    top="$(get_top)"
    treelist=($(get_treelist))
    convert_json 0 ${treelist[@]/#\//@}
}

main $@
