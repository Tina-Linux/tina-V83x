#!/bin/bash

##  top: the root path              [- /.../tina/pacakge/dragontools/tinatest -]
##  com: command in private.conf    [- stress -t 1 -]
##  pconf: path of private.conf     [- /.../tina/.../testcase/stress/io/private.conf -]
##  cpath: config path              [- /stress/io -]
##  testcase: name of testcase      [- io -]
##  condir: config dir of testcase  [- /.../tina/.../testcase/config/stress/io -]
##  conpath: path of testcase       [- /.../tina/.../testcase/config/stress/io/Config.in -]

show_help() {
    echo -e "使用说明:"
    echo -e "\033[33m    add_testcase.sh <配置文件1> [配置文件2] ...\033[0m\n"
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
    grep "^$1 *=.*$" ${pconf} | cut -d '=' -f 2- | sed -r 's/^ +//g'
}

#set_value <config-file> <config-name> <config-val>
# To menuconfig配置文件
set_value() {
    if grep ":" <<< $2 &>/dev/null; then
        local cname ctype
        ctype=${2#*:}
        cname="$(sed '{s#/#_#g; s/[a-z]/\u&/g; s/[^ [:alnum:]]/_/g}' <<< ${cpath})"
        cname="TINATEST${cname}_$(sed '{s/[a-z]/\u&/g; s/[^ [:alnum:]]/_/g}' <<< ${2%%:*})"
        if [ -n "$3" ]; then
            tac $1 | \
            sed '/private config/a\\' | \
            sed "/private config/a\\    config ${cname}" | \
            sed "/private config/a\\        ${ctype} \"${2%%:*}\"" | \
            sed "/private config/a\\        default $3" | \
            tac > $1.new
        else
            tac $1 | \
            sed '/private config/a\\' | \
            sed "/private config/a\\    config ${cname}" | \
            sed "/private config/a\\        ${ctype} \"${2%%:*}\"" | \
            tac > $1.new
        fi
        mv $1.new $1
    else
        local cname
        [ "$2" = "INFO" -o "$2" = "LIMIT" ] \
            && cname="\"$2\"" \
            || cname="_$(sed 's/[a-z]/\u&/g' <<< $2)"
        eval "sed -i '/${cname}$/{n; s/default.*$/default $3/}' $1"
    fi
}

add_depend() {
    local depends=$(get_value "DEPENDS")
    [ -z "${depends}" ] && return

    local depend
    for depend in $(sed 's/[,"]/ /g' <<< ${depends})
    do
        sed -i "3a\    select PACKAGE_${depend}" ${conpath}
    done
}

fix_value()
{
    # change config-value
    local oIFS line item value
    oIFS="${IFS}"
    IFS=$'\n'
    for line in $(grep "^[[:lower:]]" ${pconf})
    do
        IFS=" ="
        item=$(awk '{print $1}' <<< ${line})
        value="$(get_value "${item}")"
        [ "${value}" = "true" ] && value="y"
        [ "${value}" = "false" ] && value="n"
        set_value ${conpath} "${item}" "${value}"
        IFS=$'\n'
    done
    IFS="${oIFS}"

    # change INFO/LIMIT
    for item in "LIMIT" "INFO"
    do
        value=$(get_value "${item}")
        [ -n "${value}" ] && {
            [ "${value}" = "true" ] && value="y"
            [ "${value}" = "false" ] && value="n"
            set_value ${conpath} "${item}" "${value}"
        }
    done

    sed -i '/# private config/d' ${conpath}
}

create_child_config()
{
    local upper=$(sed '{s#/#_#g; s/[a-z]/\u&/g; s/[^ [:alnum:]]/_/g}' <<< ${cpath})
cat > ${conpath} << EOF
menuconfig TINATEST${upper}_ENABLE
    bool "${testcase}"
    default $([ "$(get_value "ENABLE")" = "true" ] && echo "y" || echo "n")
    ---help---
        Settings for ${cpath}
        It safe to leave a blank for any settings.
        If not setted, TinaTest will use global settings or default settings instead.

if TINATEST${upper}_ENABLE
    config TINATEST${upper}_COMMAND
        default ""
    config TINATEST${upper}_STDIN
        default ""
    config TINATEST${upper}_FSTDIN
        default ""
    config TINATEST${upper}_DATE
        default y if TINATEST_SYS_LOCAL_INFO_DATE
    config TINATEST${upper}_RESOURCE
        default y if TINATEST_SYS_LOCAL_INFO_RESOURCE
    config TINATEST${upper}_REAL_TIME_LOG
        default y if TINATEST_SYS_LOCAL_INFO_REAL_TIME_LOG
    config TINATEST${upper}_RUN_TIMES
        default 1
    config TINATEST${upper}_RUN_ALONE
        default y if TINATEST_SYS_LOCAL_LIMIT_RUN_ALONE
    config TINATEST${upper}_RUN_PARALLEL
        default y if TINATEST_SYS_LOCAL_LIMIT_RUN_PARALLEN
    config TINATEST${upper}_MAY_REBOOT
        default y if TINATEST_SYS_LOCAL_LIMIT_MAY_REBOOT
    config TINATEST${upper}_TESTCASE_RUN_ONCE_TIME
        default ""
    config TINATEST${upper}_TESTCASE_RUN_TIME
        default ""
    config TINATEST${upper}_TIMEOUT_AS_FAILED
        default n
    config TINATEST${upper}_EXIT_ONCE_FAILED
        default n
    # private config

    comment "Advanced Customized"
    config TINATEST${upper}_ADVANCED
        bool "Advanced"

    if TINATEST${upper}_ADVANCED
        config TINATEST${upper}_COMMAND
            string "command"
            ---help---
                TinaTest will run this command to start testcase.
                It is the same as shell command which followed by argumemts.
                This setting is a mark of any testcase.

        config TINATEST${upper}_STDIN
            string "stdin"
            ---help---
                Redirect a string array, which is setted in this configure, to standard input.
                For example,
                "one two three four" is equivalent to enter 4 times to testcase every run_time.
                The first time, enter "one" with new line,
                the second time, enter "two" with new line, and so on.

        config TINATEST${upper}_FSTDIN
            string "fstdin"
            ---help---
                Redirect a file, which is setted in this configure, to standard input.

        config TINATEST${upper}_INFO
            bool "INFO"
            default y

        if TINATEST${upper}_INFO
            config TINATEST${upper}_DATE
                bool "date"
                ---help---
                    Save running date and time.

            config TINATEST${upper}_RESOURCE
                bool "resource"
                ---help---
                    Save resources for each testcase.

            config TINATEST${upper}_REAL_TIME_LOG
                bool "real_time_log"
                ---help---
                    Print log of testcase real time.
                    In default, tinatest just collect all log, and print them when testcase end.
        endif

        config TINATEST${upper}_LIMIT
            bool "LIMIT"
            default y

        if TINATEST${upper}_LIMIT
            config TINATEST${upper}_RUN_TIMES
                int "run_times"
                ---help---
                    The times to run testcase.
                    For example:
                    To run this testcase for 3 times, we can set this value as 3.

            config TINATEST${upper}_RUN_ALONE
                bool "run_alone"
                ---help---
                    Wait for finishing all previous testcase and run alone before do next.

            config TINATEST${upper}_RUN_PARALLEL
                bool "run_parallel"
                ---help---
                    Run parallel for all run_times.
                    For example:
                    Testcae will run for 3 times one by one if run_times=3.
                    However, if run_parallel is setted, tinatest will creates 3 processers
                    in one time.

            config TINATEST${upper}_MAY_REBOOT
                bool "may_reboot"
                ---help---
                    It means that device will be rebooted while running testcase.
                    All states of testcase will be saved in flash, and loaded after rebooting.
                    Notices that, if may_reboot is setted, run_alone will be setted alway.

            config TINATEST${upper}_TESTCASE_RUN_ONCE_TIME
                string "testcase_run_once_time"
                ---help---
                    Time limit for testcase to run once.
                    It is in format:
                        <sec> <min> <hour> <day>
                    For example,
                    100s : 100
                    1m20s : 20 1 (or 80)
                    1d3h : 0 0 3 1 (or 0 0 28 or other)

            config TINATEST${upper}_TESTCASE_RUN_TIME
                string "testcase_run_time"
                ---help---
                    Time limit for testcase to run.
                    It is in format:
                        <sec> <min> <hour> <day>
                    For example,
                    100s : 100
                    1m20s : 20 1 (or 80)
                    1d3h : 0 0 3 1 (or 0 0 28 or other)

            config TINATEST${upper}_TIMEOUT_AS_FAILED
                bool "timeout_as_failed"
                ---help---
                    Seem to failed when testcase exit for timeout.
                    If not set, default as pass.

            config TINATEST${upper}_EXIT_ONCE_FAILED
                bool "exit_once_failed"
                ---help---
                    Stop running testcase and exit once failed,
                    even if runned times less than run_times.
        endif
    endif
endif
EOF
}

create_father_config()
{
    local base=$(basename $2)
    local upper="TINATEST$(sed '{s/[a-z]/\u&/g; s/[^ [:alnum:]]/_/g}' <<< $2)"

cat > $1 << EOF
menuconfig ${upper}
    bool "${base}"

if ${upper}
endif
EOF
}

recurse_father_config()
{
    local dir=$(dirname $1)
    local base=$(basename $1)
    local cur="${top}/config${dir}/Config.in"

    [ "${dir}" = "/" ] && return

    [ -f "${cur}" ] || create_father_config ${cur} ${dir}
    grep -w "source ${base}/Config.in$" ${cur} &>/dev/null || \
        sed -i "/endif/i\\\\    source ${base}/Config.in" ${cur}

    recurse_father_config ${dir}
}

new_config()
{
    create_child_config
    recurse_father_config ${cpath}
}

add_config() {
    testcase="$(basename ${cpath})"
    condir="${top}/config${cpath}"
    conpath="${condir}/Config.in"
    mkdir -p ${condir}

    new_config
    fix_value
}

add_main() {
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
            && echo -e "\033[31mMismatch path from ${pconf}\033[0m" \
            && continue

        # get command
        com=$(get_value "command")
        [ -z "${com}" ] \
            && echo -e "\033[31mMismatch command in ${pconf}\033[0m" \
            && continue

        # do add config
        add_config
        add_depend
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
    [ "$#" -lt 1 ] && show_help && exit 0
    # get top path
    get_top
    # do add
    add_main $@
    # make it built after adding testcase
    touch ${top}/Makefile
    echo "All Done!"
}

main $@
