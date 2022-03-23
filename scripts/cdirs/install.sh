#!/bin/bash

cdirs_src="cdirs.sh"
cdirs_default="cdirs_default.tina"
cdirs_config="cdirsrc.tina"

# get_absolute_path <path>
get_absolute_path() {
    local pwd_path="${PWD}"
    local para_path="$1"

    # deal with word like - ~ . ..
    if [ "${para_path}" = "-" ]; then
        echo "${OLDPWD}"
        return 0
    elif [ "${para_path}" = "." ]; then
        echo "${PWD}"
        return 0
    elif [ "${para_path}" = ".." ]; then
        echo "${PWD%/*}"
        return 0
    elif [ "${para_path}" = "~" ]; then
        echo "${HOME}"
        return 0
    elif [ "${para_path:0:1}" = "~" ]; then
        para_path="${HOME}${para_path:1}"
    fi

    # delete last letter /
    para_path=$(echo "${para_path}" | sed 's/\(.*\)\/$/\1/g')

    # deal with word like ./ ../
    while [ -n "$(echo "${para_path}" | egrep "\./|\.\./")" ]
    do
        if [ "${para_path%%/*}" = ".." ]; then
            pwd_path="${pwd_path%/*}"
        elif [ ! "${para_path%%/*}" = "." ]; then
            pwd_path="${pwd_path}/${para_path%%/*}"
        fi
        para_path="${para_path#*/}"
    done

    if [ ! "${pwd_path}" = "${PWD}" ]; then
        echo "${pwd_path}/${para_path}"
    elif [ -d "${para_path}" ] && [ ! "${para_path:0:1}" = "/" ]; then
        echo "${PWD}/${para_path}"
    else
        echo "${para_path}"
    fi
}

check_cdirs() {
    echo -n "CHECKING cdirs ... "
    src="`pwd`/${cdirs_src}"
    [ ! -f ${src} ] && {
        src="$(get_absolute_path $(dirname $0))/${cdirs_src}"
        [ ! -f ${src} ] && {
            echo -e "failed : \033[31mnot find cdirs.sh\033[0m"
            exit 1
        }
    }

    srcpath=$(dirname $src)
    rootpath=$(dirname $(dirname ${srcpath}))
    f_install="${rootpath}/build/envsetup.sh"
    [ -f "${rootpath}/build/envsetup.sh" ] || {
        echo -e "failed : \033[31mnot find ${rootpath}/build/envsetup.sh\033[0m"
        exit 1
    }

    src_default="${srcpath}/${cdirs_default}"
    src_config="${srcpath}/${cdirs_config}"
    [ -f ${src_default} ] || {
        echo -e "failed : \033[31mnot find ${src_default}\033[0m"
        exit 1
    }
    [ -f ${src_config} ] || {
        echo -e "failed : \033[31mnot find ${src_config}\033[0m"
        exit 1
    }
    chmod u+x ${src}
    echo "ok : src-${src}"
}

print_help() {
    echo "Usage: ./install.sh [-h|--help] [--uninstall] [--unreplace-cd]"
}

uninstall() {
    [ -f ${f_install} ] && sed -i '/set for cdirs/,/end for cdirs/d' ${f_install}
    [ -f ~/.bash_logout ] && sed -i '/set for cdirs/,/end for cdirs/d' ~/.bash_logout
    [ -f ~/.bashrc ] && sed -i '/set by cdirs/,/end by cdirs/d' ~/.bashrc
}

install_bashrc() {
cat >> ~/.bashrc <<EOF
# == set by cdirs ==
setenv-tina() {
	cd ${rootpath}
	echo -e "\033[0;34;1m======== source build/envsetup.sh ========\033[0m"
    source ${f_install} >/dev/null
	echo

	echo -e "\033[0;34;1m======== lunch ========\033[0m"
	local platform="\$1"
	local default_platform="r40_m2ultra-tina"
	[ -z "\${platform}" ] && {
		local menu=(\$(echo \${LUNCH_MENU_CHOICES[@]} | sed 's/ /\n/g' | sort))
		echo "Lunch Menu:"
		echo \${menu[@]} | sed 's/ /\n/g' | awk '{print "    " NR ".\t" \$1}'
		while true
		do
			read -p "Your Choice Num [default \${default_platform}]: " platform
			[ -z "\${platform}" ] && {
				platform="\${default_platform}"
				break
			} || {
				echo \${platform} | egrep "[[:digit:]]+" &>/dev/null && {
					platform="\${menu[platform-1]}"
					[ -z "\${platform}" ] \
						&& echo -e "\033[0;35;1mInvalid lunch combo\033[0m" \
						|| break
				}
			}
		done
	}
	lunch \${platform}
}
# == end by cdirs ==
EOF
}

install() {
    echo -n "INSTALLING cdirs ... "
    #
    sed -i '241a \
    # == set for cdirs ==\
    gmpy_cdirs_load_config\
    gmpy_cdirs_load_global_labels &>/dev/null\
    # == end for cdirs ==' ${f_install}
cat >> ${f_install} <<EOF
# == set for cdirs ==
[ "\$(type -t cd)" = "alias" ] && unalias cd
wait
source ${src} $([ -z "${not_replace_cd}" ] && echo "--replace-cd")
gmpy_cdirs_clear_mark >/dev/null
gmpy_cdirs_set_env "no_global" "0" "," "${rootpath}"
# == end for cdirs ==
EOF
cat >> ~/.bash_logout <<EOF
# == set for cdirs ==
[ -n "\${gmpy_cdirs_env}" ] && rm \${gmpy_cdirs_env}
# == end for cdirs ==
EOF
    install_bashrc
    echo "ok : install-${f_install}"
    echo -e "\033[32msee more $([ -z "${not_replace_cd}" ] \
        && echo "cd|")cdir|setdir|lsdir|cldir --help\033[0m"
}


if [ "$#" -gt 1 ]; then
    print_help
    exit 1
elif [ "$#" -eq 1 ]; then
    case "$1" in
        --uninstall)
            check_cdirs
            uninstall
            echo -e "\033[31mcdirs has unistalled, have a fun day\033[0m"
            exit 0
            ;;
        --unreplace-cd)
            not_replace_cd=1
            shift
            ;;
        *)
            print_help
            exit 0
            ;;
    esac
fi

check_cdirs
uninstall
install
