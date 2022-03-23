#!/bin/bash

## cpath: config path, such as /stress/reboot
## pconf: path of private.conf, such as ../testcase/stress/io/private.conf
## top: the root path of tinatest, such as tina/pacakge/dragontools/tinatest

show_help() {
    echo -e "使用说明:"
    echo -e "\033[33m    del_testcase.sh <配置文件1> [配置文件2] ...\033[0m\n"
    echo -e "配置文件:"
    echo -e "    记录新用例的路径以及默认配置值,一行一条键值对,格式为:"
    echo -e "\033[33m              <配置项>[:类型] = <配置值>\033[0m"
    echo -e "    \033[33m[配置项]\033[0m: 包含PATH/ENABLE/INFO/LIMIT/DEPENDS/DATA和测试用例的所有配置项(例如:command,run_times,run_alone等)"
    echo -e "    其中:"
    echo -e "        PATH: 测试用例在配置树中的绝对路径(字符串)"
    echo -e "        ENABLE: 默认是否使能此用例(bool)"
    echo -e "        INFO: 默认是否使能所有的 局部信息 配置项(bool)"
    echo -e "        LIMIT: 默认是否使能所有的 局部限制 配置项(bool)"
    echo -e "        DEPENDS: 测试用例依赖的第三方应用包(string),多个包之间以逗号间隔"
    echo -e "                 格式: \"<依赖的软件包1>[,<依赖的软件包2>,...]"
    echo -e "                 例如 /stress/rw/rw-auto 依赖 rwcheck 软件包, 则 DEPENDS=\"rwcheck\""
    echo -e "        DATA: 测试用例需要使用的一些数据文件,文件必须保存在测试用例源码目录"
    echo -e "              格式: \"<数据文件1>[,<数据文件2>...]\""
    echo -e "              例如: /base/production/headphonetester需要文件s16le_44100_stereo.wav"
    echo -e "                    则 DATA = \"s16le_44100_stereo.wav\""
    echo -e "        \033[32m[大写字母为特定配置项][无指定类型的小写字母为用例属性项][指定类型的小写字母为私有配置项]\033[0m"
    echo -e "    \033[33m[类型]\033[0m: \033[31m(私有配置项才需要)\033[0m为mjson支持的数据类型,包括:int/double/true/false/string/array"
    echo -e "    \033[33m[配置值]\033[0m: 支持字符串/字符串数组/整数/浮点数/bool(见示例)"
    echo -e "    其中:"
    echo -e "        1) \033[31m字符串必须用双引号括起来\033[0m"
    echo -e "        2) 字符串数组以字符串形式表示,字符串之间以空格间隔"
    echo -e "        4) 字符串内若双引号\\转义字符等,需要有双重转义, 例如: command = \"echo \\\\\\\\\\\\\"test\\\\\\\\\\\\\"\" 表示echo \"test\""
    echo -e "        4) 每一行开头不能有空格/Tab等"
    echo -e "\n示例如下:"
    echo -e "    |PATH = /demo/demo-c"
    echo -e "    |ENABLE = false"
    echo -e "    |INFO = true"
    echo -e "    |command = \"demo-c\""
    echo -e "    |run_times = 10"
    echo -e "    |run_alone = false"
    echo -e "    |workers:int = 2"
    echo -e "    |words:string = \"test\""
    echo -e "    |right:bool = true"
    echo -e "    |str_array:array = \"one two three\""
}

#get_value <config-name>
# From 配置文件
get_value() {
    grep "^$1 *=.*$" ${oPWD}/${pconf} | cut -d '=' -f 2- | sed -r 's/^ +//g'
}

recurse_father_config()
{
    local dir=$(dirname $1)
    local base=$(basename $1)
    local cur="${top}/config${dir}/Config.in"

    [ "${dir}" = "/" ] && return

    [ -f "${cur}" ] || return 1
    # delete source
    sed -i "/source ${base}\/Config\.in/d" ${cur}
    # remove emtpy Cofnig.in and dir
    ! grep -q "source .*\.in" ${cur} \
        && rm -rf $(dirname ${cur}) \
        && recurse_father_config ${dir}
    return 0
}

del_config() {
    condir="${top}/config${cpath}"

    # delete child Config.in
    [ -d "${condir}" ] && rm -rf ${condir}

    # fix father Config.in
    recurse_father_config ${cpath}
}

del_main() {
    for pconf in $@
    do
        # get and check private.conf
        pconf="$(cd $(dirname ${pconf}) && pwd)/$(basename ${pconf})"
        [ ! -f "${pconf}" ] \
            && echo -e "\033[31mNot find \"${pconf}\"\033[0m" \
            && continue

        # get and check config-path
        cpath=$(dirname ${pconf} | sed 's#.*tinatest/testcase##g')
        [ -z "${cpath}" ] \
            && echo "\033[31mMismatch path from ${pconf}\033[0m" \
            && continue

        # do del config
        del_config
    done
}

get_top()
{
    top=`pwd`
    while [ ! "${top}" = "/" ]
    do
        [ -f "${top}/Makefile" ] \
            && egrep -w "PKG_NAME *[:]?= *tinatest" ${top}/Makefile >/dev/null \
            && break
        top=$(dirname ${top})
    done
    [ "${top}" = "/" ] && echo "Get top failed" && exit
}

main() {
    local opts
    opts="$(getopt -l "help" -o "h" -- $@)" || return 1
    eval set -- "${opts}"
    while true
    do
        case "$1" in
            -h|--help)
                show_help
                exit 0
                ;;
            --)
                shift
                break
        esac
    done

    # check usage
    [ "$#" -lt 1 ] && show_help
    # get top path
    get_top
    # do del
    del_main $@
    # make it built after adding testcase
    touch ${top}/Makefile
    echo "All Done!"
}

main $@
