cdirs_src="install.sh"

check_install() {
    echo -n "CHECKING cdirs ... "
    src="`pwd`/${cdirs_src}"
    [ ! -f ${src} ] && {
        src="$(get_absolute_path $(dirname $0))/${cdirs_src}"
        [ ! -f ${src} ] && {
            echo -e "failed : \033[31mnot find cdirs.sh\033[0m"
            exit 1
        }
    }
    chmod 755 ${src}
    echo "ok : src-${src}"
}

check_install
eval "${src} --uninstall"
