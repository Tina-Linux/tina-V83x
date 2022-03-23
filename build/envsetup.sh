function hmm() {
cat <<EOF
Invoke ". build/envsetup.sh" from your shell to add the following functions to your environment:

== before all ==
- lunch:        lunch <product_name>-<build_variant>

== build project ==
- m:            Make from the top of the tree.
- mm:           Build package in the current directory, but not their dependencies.
- mmb:          Clean and build package in the current directory, but not their dependencies.
- p:            Pack from the top of the tree.
- pd:           Pack card0 from the top of the tree.
- mp:           Make and pack from the top of the tree
- mpd:          Make and pack card0 from the top of the tree
- mboot:        Build boot0 and uboot, including uboot for nor.
- mboot0:       Just build boot0.
- muboot:       Build uboot, including uboot for nor.
- muboot_nor:   Just build uboot for nor.
- mkernel:      Build kernel.
- mlibc:        Build c library.

== jump directory ==
- croot:    Jump to the top of the tree.
- cboot:    Jump to uboot.
- cboot0:   Jump to boot0.
- cdts:     Jump to device tree.
- cbin:     Jump to uboot/boot0 bin directory.
- ckernel:  Jump to kernel.
- cdevice:  Jump to target.
- ccommon:  Jump to platform common.
- cconfigs: Jump to configs of target.
- cout:     Jump to out directory of target.
- ctarget:  Jump to target of compile directory.
- crootfs:  Jump to rootfs of compile directory.
- ctoolchain: Jump to toolchain directory.
- callwinnerpk: Jump to package allwinner directory.
- ctinatest:  Jump to tinateset directory.
- godir:    Go to the directory containing a file.

== grep file ==
- cgrep:    Greps on all local C/C++ files.

Look at the source to view more functions. The complete list is:
EOF
    T=$(gettop)
    local A
    A=""
    for i in `cat $T/build/envsetup.sh | sed -n "/^[ \t]*function /s/function \([a-z_]*\).*/\1/p" | sort | uniq`; do
      A="$A $i"
    done
    echo $A
}

function gettop
{
    local TOPFILE=build/envsetup.sh
    if [ -n "$TINA_TOP" -a -f "$TINA_TOP/$TOPFILE" ] ; then
        # The following circumlocution ensures we remove symlinks from TOP.
        (\cd $TINA_TOP; PWD= /bin/pwd)
    else
        if [ -f $TOPFILE ] ; then
            # The following circumlocution (repeated below as well) ensures
            # that we record the true directory name and not one that is
            # faked up with symlink names.
            PWD= /bin/pwd
        else
            local here="${PWD}"
            while [ "${here}" != "/" ]; do
                if [ -f "${here}/${TOPFILE}" ]; then
                    (\cd ${here}; PWD= /bin/pwd)
                    break
                fi
                here="$(dirname ${here})"
            done
        fi
    fi
}

export PLAT_PATH=sun8iw19p1
PLAT_PATH=sun8iw19p1
function cdmpp ()
{
    local T=$(gettop)
    cd $T/softwinner/eyesee-mpp/middleware/$PLAT_PATH
}

function bbmpp ()
{
    local T=$(gettop)
    cd $T/package/allwinner/eyesee-mpp/middleware
    mmb
}

function add_lunch_combo()
{
    local c
    for c in ${LUNCH_MENU_CHOICES[@]} ; do
        if [ "$1" = "$c" ] ; then
            return
        fi
    done
    LUNCH_MENU_CHOICES=(${LUNCH_MENU_CHOICES[@]} $1)
}

function print_lunch_menu()
{
    local uname=$(uname)
    echo
    echo "You're building on" $uname
    echo
    echo "Lunch menu... pick a combo:"

    local i=1
    local choice
    for choice in ${LUNCH_MENU_CHOICES[@]}
    do
        echo "     $i. $choice"
        i=$(($i+1))
    done
    echo
}

# check to see if the supplied product is one we can build
function check_platform()
{
    local T=$(gettop)
    [ -d "$T/target/allwinner" ] || return 1

    local v
    for v in ${PLATFORM_CHOICES}
    do
        [ "$v" = "$1" ] && return 0
    done
    return 1
}

function check_product()
{
    T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    TARGET_PRODUCT=$1 \
    TARGET_BUILD_VARIANT= \
    TARGET_BUILD_TYPE= \
    TARGET_BUILD_APPS= \
    get_build_var TARGET_DEVICE > /dev/null
}

# check to see if the supplied variant is valid
function check_variant()
{
    for v in ${VARIANT_CHOICES}
    do
        [ "$v" = "$1" ] && return 0
    done
    return 1
}

# Get the exact value of a build variable.
function get_build_var()
{
    T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    (\cd $T; CALLED_FROM_SETUP=true BUILD_SYSTEM=build \
      command make --no-print-directory -f build/config.mk dumpvar-$1)
}

function get_chip
{
    local T=$(gettop)
    [ -z "$T" ] && return -1
    [ -z "${TARGET_PLATFORM}" ] && return -1
    [ -z "${TARGET_KERNEL_VERSION}" ] && return -1

    local longan_kernel_config=$T/target/allwinner/${TARGET_BOARD}/config-${TARGET_KERNEL_VERSION}
    local tina_kernel_config=$T/device/config/chips/${TARGET_PLATFORM}/configs/${TARGET_BOARD#*-}/linux/config-${TARGET_KERNEL_VERSION}
    [ -e "${longan_kernel_config}" ] && awk -F'[_=]' '/CONFIG_ARCH_SUN.*P.*=y/{print tolower($3)}' "${longan_kernel_config}" | head -n 1 && return
    [ -e "${tina_kernel_config}" ] && awk -F'[_=]' '/CONFIG_ARCH_SUN.*P.*=y/{print tolower($3)}' "${tina_kernel_config}" | head -n 1 && return
    return -1
}

function get_uboot
{
    local T=$(gettop)
    [ -z "$T" ] && return -1
    [ -z "${TARGET_PRODUCT}" ] && return -1
    local f="$T/target/allwinner/${TARGET_PRODUCT/_/-}/Makefile"
    [ -f "$f" ] || return -1

    awk -F":=" '/UBOOT_PATCHVER/{print $2}' $f
}

function get_kernel
{
    local T=$(gettop)
    [ -z "$T" ] && return -1
    [ -z "${TARGET_PRODUCT}" ] && return -1
    local f="$T/target/allwinner/${TARGET_PRODUCT/_/-}/Makefile"
    [ -f "$f" ] || return -1

    awk -F":=" '/KERNEL_PATCHVER/{print $2}' $f
}

function get_arch
{
    local T=$(gettop)
    [ -z "$T" ] && return -1
    [ -z "${TARGET_PRODUCT}" ] && return -1
    local f="$T/target/allwinner/${TARGET_PRODUCT/_/-}/Makefile"
    [ -f "$f" ] || return -1

    awk -F":=" '/ARCH/{print $2}' $f
}

function parse_boardconfig()
{
    export LICHEE_CHIP_CONFIG_DIR=${TINA_BUILD_TOP}/device/config/chips/${TARGET_PLATFORM}

    local special_config="$T/device/config/chips/${TARGET_PLATFORM}/configs/${TARGET_PLAN}/BoardConfig.mk"
    local config_list=""

    config_list=($special_config)

    local fetch_list=""
    local fpare_list=""
    for f in ${config_list[@]}; do
        if [ -f $f ]; then
            fetch_list=(${fetch_list[@]} $f)
            fpare_list="$fpare_list -f $f"
        fi
    done

    local cfgkeylist=(
        LICHEE_BUSSINESS
        )

    local cfgkey=""
    local cfgval=""
    for cfgkey in ${cfgkeylist[@]}; do
        if [ -n "$fpare_list" ]; then
            cfgval="$(echo '__unique:;@echo ${'"$cfgkey"'}' | command make -f - $fpare_list --no-print-directory __unique)"
        else
            cfgval=""
        fi
        export $cfgkey=$cfgval
    done

}

function lunch
{
    # get last platfrom as default platform
    local T="$(gettop)"
    local last
    if [ -f "$T/.config" ]; then
        #last="$(awk -F[=_] '/CONFIG_TARGET_[a-z_0-9]*[^_]=y/{print $3 "_" $4; exit}' ${T}/.config)"
        last="$(sed -n -r '/CONFIG_TARGET_[a-zA-Z_0-9]*[^_]=y/{s/CONFIG_TARGET_([a-zA-Z_0-9]*[^_])=y/\1/;p;q}' ${T}/.config)"
    fi

    # select platform
    local select
    if [ "$1" ] ; then
        select=$1
    else
        print_lunch_menu
        echo -n "Which would you like?"
        [ -n "${last}" ] && echo -n " [Default ${last}]"
        echo -n ": "
        read select
    fi
    if [ -z "${select}" ]; then
        select="${last}-tina"
    elif (echo -n $select | grep -q -e "^[0-9][0-9]*$"); then
        [ $select -le ${#LUNCH_MENU_CHOICES[@]} ] \
            && select=${LUNCH_MENU_CHOICES[$(($select-1))]}
    elif (echo -n $select | grep -q -e "^[^\-][^\-]*-[^\-][^\-]*$"); then
        select="$select"
    else
        echo
        echo "Invalid lunch combo: $select" >&2
        return 1
    fi

    # check platform
    local platform=$(echo -n $select | sed -e "s/_.*$//")
    check_platform ${platform}
    if [ $? -ne 0 ]; then
        echo
        echo "** Don't have a platform spec for: '$platform'" >&2
        echo "** Must be one of ${PLATFORM_CHOICES}" >&2
        echo "** Do you have the right repo manifest?" >&2
        platform=
    fi

    # check product
    local product=$(echo -n $select | sed -e "s/-.*$//")
    check_product $product
    if [ $? -ne 0 ]
    then
        echo
        echo "** Don't have a product spec for: '$product'" >&2
        echo "** Do you have the right repo manifest?" >&2
        product=
    fi

    local variant=$(echo -n $select | sed -e "s/^[^\-]*-//")
    check_variant $variant
    if [ $? -ne 0 ]
    then
        echo
        echo "** Invalid variant: '$variant'" >&2
        echo "** Must be one of ${VARIANT_CHOICES}" >&2
        variant=
    fi

    [ -z "$product" -o -z "$variant" -o -z "$platform" ] && return 1

    export TARGET_PRODUCT=$product
    export TARGET_PLATFORM=$platform
    export TARGET_BOARD=$(get_build_var TARGET_DEVICE)
    export TARGET_PLAN=${TARGET_BOARD#*-}
    export TARGET_BUILD_VARIANT=$variant
    export TARGET_BUILD_TYPE=release
    export TARGET_KERNEL_VERSION=$(get_kernel)
    export TARGET_UBOOT=u-boot-$(get_uboot)
    export TARGET_CHIP=$(get_chip)
    export TINA_TARGET_ARCH=$(get_arch)
    export TINA_BUILD_TOP=$(gettop)
    #for plat_config.sh
    parse_boardconfig
    export LICHEE_PACK_OUT_DIR=${TINA_BUILD_TOP}/out/${TARGET_BOARD}/image
    echo "============================================"
    echo "TINA_BUILD_TOP=${TINA_BUILD_TOP}"
    echo "TINA_TARGET_ARCH=${TINA_TARGET_ARCH}"
    echo "TARGET_PRODUCT=${TARGET_PRODUCT}"
    echo "TARGET_PLATFORM=${TARGET_PLATFORM}"
    echo "TARGET_BOARD=${TARGET_BOARD}"
    echo "TARGET_PLAN=${TARGET_PLAN}"
    echo "TARGET_BUILD_VARIANT=${TARGET_BUILD_VARIANT}"
    echo "TARGET_BUILD_TYPE=${TARGET_BUILD_TYPE}"
    echo "TARGET_KERNEL_VERSION=${TARGET_KERNEL_VERSION}"
    echo "TARGET_UBOOT=${TARGET_UBOOT}"
    echo "TARGET_CHIP=${TARGET_CHIP}"
    echo "============================================"

    export BUILD_ENV_SEQUENCE_NUMBER=10
    # With this environment variable new GCC can apply colors to warnings/errors
    export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'

    rm -rf tmp

    [ -e $T/target/allwinner/${TARGET_BOARD}/${TARGET_PRODUCT}-setup.sh ] \
        && source $T/target/allwinner/${TARGET_BOARD}/${TARGET_PRODUCT}-setup.sh
}

# Tab completion for lunch.
function _lunch
{
    local cur prev
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    COMPREPLY=( $(compgen -W "${LUNCH_MENU_CHOICES[*]}" -- ${cur}) )
    return 0
}

# Tab completion for m.
function _m
{
    local T=$(gettop)
    [ -z "$T" ] && return
    [ -f "$T/build/toplevel.mk" ] || return

    local cur prev list
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    list="$(grep "[^\.].*config:" $T/build/toplevel.mk | awk -F: '{print $1}')"
    COMPREPLY=( $(compgen -W "${list}" -- ${cur}) )
}

function envsetup
{
    if [ "x$SHELL" != "x/bin/bash" ]; then
        case `ps -o command -p $$` in
            *bash*)
                ;;
            *)
                echo -n "WARNING: Only bash is supported, "
                echo "use of other shell would lead to erroneous results"
                ;;
        esac
    fi

    # check top of SDK
    if [ ! -f "${PWD}/build/envsetup.sh" ]; then
        echo "ERROR: Please source envsetup.sh in the root of SDK"
        return -1
    else
        export TINA_TOP="$(PWD= /bin/pwd)"
    fi

    # reset these variables.
    # LUNCH_MENU_CHOICES will be built up again when the vendorsetup.sh are included
    unset LUNCH_MENU_CHOICES
    export LUNCH_MENU_CHOICES
    export VARIANT_CHOICES=tina
    export PLATFORM_CHOICES="$(ls $(gettop)/target/allwinner 2>/dev/null \
        | awk -F- '/.*-.*/{print $1}' | sort | uniq)"

    # Execute the contents of any vendorsetup.sh files we can find.
    local vendors vendor
    verdors="$(find -L target -maxdepth 4 -name 'vendorsetup.sh' 2>/dev/null | sort)"
    for verdor in ${verdors}
    do
        source ${verdor}
    done
    echo "Setup env done! Please run lunch next."

    # completion
    complete -F _lunch lunch
    complete -F _m m
}

function m
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    (\cd $T && make $@)
}

function mm
{
	local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

	$T/scripts/mm.sh $T $*
}

function mmb
{
    mm -B -j32
}

function p
{
	local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    (\cd $T && pack $@)
}

function pd
{
    p -d
}

function mp
{
    m $@ && p
}

function mpd
{
    m $@ && pd
}

# Build brandy(uboot,boot0,fes) if you want.
function build_boot()
{
	local T=$(gettop)
	local chip=${TARGET_CHIP}
	local cmd=$1
	local o_option=$2
	local platform
	local bin_dir

	echo $TARGET_PRODUCT $TARGET_PLATFORM $TARGET_BOARD
	if [ -z "$TARGET_BOARD" -o -z "$TARGET_PLATFORM" ]; then
		echo "Please use lunch to select a target board before build boot."
		return 1
	fi

	if [ "x$chip" = "x" ]; then
		echo "platform($TARGET_PLATFORM) not support"
		return 1
	fi

	platform=${chip}

	bin_dir="device/config/chips/${TARGET_PLATFORM}/bin"
	if [ "${TARGET_UBOOT}" = "u-boot-2018" ]; then
		\cd $T/lichee/brandy-2.0/
		[ x"$o_option" = x"uboot" -a -f "u-boot-2018/configs/${TARGET_BOARD}_defconfig" ] && {
			platform=${TARGET_BOARD}
			bin_dir="device/config/chips/${TARGET_PLATFORM}/configs/${TARGET_BOARD}/bin"
			mkdir -p $T/${bin_dir}
		}
	else
		\cd $T/lichee/brandy/
	fi

	echo "build_boot platform:$platform o_option:$o_option"
	if [ x"$o_option" != "x" ]; then
		TARGET_BIN_DIR=${bin_dir} ./build.sh -p $platform -o $o_option
	else
		TARGET_BIN_DIR=${bin_dir} ./build.sh -p $platform
	fi
	if [ $? -ne 0 ]; then
		echo "$cmd stop for build error in brandy, Please check!"
		\cd - 1>/dev/null
		return 1
	fi
	\cd - 1>/dev/null
	echo "$cmd success!"
	return 0
}

function muboot
{
    (build_boot muboot uboot)
}

function mboot
{
    (build_boot muboot uboot)
    (build_boot mboot0 boot0)
}

function mboot0
{
    (build_boot mboot0 boot0)
}

function muboot_nor
{
    (build_boot muboot_nor uboot_nor)
}

function mkernel
{
    m target/allwinner/install $@
}

function mlibc() {
	local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

	$T/scripts/mlibc.sh $T $*
}

function make_img_md5(){
    #$1: target image
    md5sum $1 | awk '{print $1}' > $1.md5
}

function ota_save_files()
{
    [ $# -lt 1 ] && echo "usage:ota_save_files file_dir" && return 1
    local target_dir=$1
    echo target_dir:"$target_dir"
    [ ! -d "$target_dir" ] && echo target_dir:"$target_dir" not exit!! &&  return 1

    local T=$(gettop)
    local BIN_DIR=$T/out/${TARGET_BOARD}
    local boot_img="$(readlink -f "$BIN_DIR"/image/boot.fex)"
    local rootfs_img="$(readlink -f "$BIN_DIR"/image/rootfs.fex)"
    local recovery_img="$(readlink -f "$BIN_DIR"/image/recovery.fex)"
    #uboot and boot0
    local boot_package_img="$(readlink -f "$BIN_DIR"/image/boot_package.fex)"
    local boot0_nand_img="$(readlink -f "$BIN_DIR"/image/boot0_nand.fex)"
    local boot0_sdcard_img="$(readlink -f "$BIN_DIR"/image/boot0_sdcard.fex)"

    rm -f "$target_dir"/*.img "$target_dir"/*.md5 "$target_dir"/*.signature
    [ -f "$boot_img" ] \
        && cp "$boot_img" "$target_dir"/boot.img \
        && make_img_md5 "$target_dir"/boot.img
    [ -f "$rootfs_img" ] \
        && cp "$rootfs_img" "$target_dir"/rootfs.img \
        && make_img_md5 "$target_dir"/rootfs.img
    [ -f "$recovery_img" ] \
        && cp "$recovery_img" "$target_dir"/recovery.img \
        && make_img_md5 "$target_dir"/recovery.img
    [ -f "$boot_package_img" ] \
        && cp "$boot_package_img" "$target_dir"/boot_package.img \
        && make_img_md5 "$target_dir"/boot_package.img
    [ -f "$boot0_nand_img" ] \
        && cp "$boot0_nand_img" "$target_dir"/boot0_nand.img \
        && make_img_md5 "$target_dir"/boot0_nand.img
    [ -f "$boot0_sdcard_img" ] \
        && cp "$boot0_sdcard_img" "$target_dir"/boot0_sdcard.img \
        && make_img_md5 "$target_dir"/boot0_sdcard.img
    ls -l "$target_dir"
}

function make_ota_image(){
    local T=$(gettop)
    local chip=sunxi
    local need_usr=0
    local make_ota_fail=0
    [ x$CHIP = x"sun5i" ] && chip=sun5i
    local BIN_DIR=$T/out/${TARGET_BOARD}
    local OTA_DIR=$BIN_DIR/ota
    mkdir -p $OTA_DIR
    print_red "build ota package"
    grep "CONFIG_SUNXI_SMALL_STORAGE_OTA=y" $T/target/allwinner/${TARGET_BOARD}/defconfig \
        && need_usr=1
    #target image
    target_list="$BIN_DIR/boot.img $BIN_DIR/rootfs.img $BIN_DIR/usr.img"
    if [ $need_usr -eq 0 ];then
        target_list="$BIN_DIR/boot.img $BIN_DIR/rootfs.img"
    fi
    [ -n $1 ] && [ x$1 = x"--force" ] && rm -rf $target_list
    for i in $target_list; do
        if [ ! -f $i ]; then
            img=${i##*/}
            print_red "$i is not exsit! rebuild the image."
            make -j16
            if [ $? -ne 0 ]
            then
                print_red "make $img file! make_ota_image fail!"
                make_ota_fail=1
            fi
            break
        fi
    done

    rm -rf $OTA_DIR/target_sys
    mkdir -p $OTA_DIR/target_sys
    dd if=$BIN_DIR/boot.img of=$OTA_DIR/target_sys/boot.img
    make_img_md5 $OTA_DIR/target_sys/boot.img
    dd if=$BIN_DIR/rootfs.img of=$OTA_DIR/target_sys/rootfs.img
    make_img_md5 $OTA_DIR/target_sys/rootfs.img

    local storage_type_nor=0
    local f="$T/device/config/chips/${TARGET_PLATFORM}/configs/${TARGET_PLAN}/sys_config.fex"
    local B="$( awk -F"=" '/^storage_type/{print $2}' $f | sed 's/^[ \t]*//g' )"
    case $B in
        *0 | *5)
            local boot0_img=boot0_nand.fex
            ;;
        *1 | *2 | *4)
            local boot0_img=boot0_sdcard.fex
            ;;
        3)
            local boot0_img=boot0_spinor.fex
            storage_type_nor=1
            ;;
        *)
            echo "###storage type error###"
            ;;
    esac
    rm -rf $OTA_DIR/boot0_sys
    [ -n $boot0_img ] && {
        mkdir -p $OTA_DIR/boot0_sys
        dd if=$BIN_DIR/image/$boot0_img of=$OTA_DIR/boot0_sys/boot0.img
        make_img_md5 $OTA_DIR/boot0_sys/boot0.img
    }
    local U="$(get_uboot)"
    if [[ "$U" =~ "2011" ]]; then
        local uboot_img=u-boot.fex
    else
        if [ x"$storage_type_nor" = x"1" ]; then
            local uboot_img=boot_package_nor.fex
        else
            local uboot_img=boot_package.fex
        fi
    fi
    rm -rf $OTA_DIR/uboot_sys
    mkdir -p $OTA_DIR/uboot_sys
    dd if=$BIN_DIR/image/$uboot_img of=$OTA_DIR/uboot_sys/uboot.img
    make_img_md5 $OTA_DIR/uboot_sys/uboot.img

    if [ $need_usr -eq 1 ];then
        rm -rf $OTA_DIR/usr_sys
        mkdir -p $OTA_DIR/usr_sys
        dd if=$BIN_DIR/usr.img of=$OTA_DIR/usr_sys/usr.img
        make_img_md5 $OTA_DIR/usr_sys/usr.img
    fi

    grep -v -e CONFIG_TARGET_ROOTFS_INITRAMFS \
        $T/target/allwinner/${TARGET_BOARD}/defconfig_ota > .config_ota
    echo 'CONFIG_TARGET_ROOTFS_INITRAMFS=y' >> .config_ota
    echo 'CONFIG_TARGET_AW_OTA_INITRAMFS=y' >> .config_ota
    echo 'CONFIG_TARGET_INITRAMFS_COMPRESSION_XZ=y' >> .config_ota

    make V=s -j16 TARGET_CONFIG=.config_ota
    if [ $? -ne 0 ]
    then
        print_red "make_ota_image fail!"
        make_ota_fail=1
    fi

    rm -rf $OTA_DIR/ramdisk_sys
    mkdir -p $OTA_DIR/ramdisk_sys

    local recovery_img_source="boot_initramfs.img"
    local recovery_img_dest="boot_initramfs.img"
    if grep -q CONFIG_SUNXI_BOOT_IMAGE_NAME_SUFFIX_RECOVERY=y .config_ota; then
        echo "have CONFIG_SUNXI_BOOT_IMAGE_NAME_SUFFIX_RECOVERY"
        echo "use boot_initramfs_recovery.img"
        recovery_img_source="boot_initramfs_recovery.img"
    fi

    dd if=$BIN_DIR/$recovery_img_source of=$OTA_DIR/ramdisk_sys/$recovery_img_dest
    make_img_md5 $OTA_DIR/ramdisk_sys/$recovery_img_dest

###########################################################
    rm -rf $OTA_DIR/package_sys
    mkdir -p $OTA_DIR/package_sys

    dd if=$OTA_DIR/ramdisk_sys/$recovery_img_dest of=$OTA_DIR/package_sys/recovery.img
    make_img_md5 $OTA_DIR/package_sys/recovery.img
    dd if=$OTA_DIR/target_sys/boot.img of=$OTA_DIR/package_sys/boot.img
    make_img_md5 $OTA_DIR/package_sys/boot.img
    dd if=$OTA_DIR/target_sys/rootfs.img of=$OTA_DIR/package_sys/rootfs.img
    make_img_md5 $OTA_DIR/package_sys/rootfs.img

    dd if=$OTA_DIR/boot0_sys/boot0.img of=$OTA_DIR/package_sys/boot0.img
    make_img_md5 $OTA_DIR/package_sys/boot0.img
    dd if=$OTA_DIR/uboot_sys/uboot.img of=$OTA_DIR/package_sys/uboot.img
    make_img_md5 $OTA_DIR/package_sys/uboot.img
    if [ -f $BIN_DIR/image/bootlogo.fex ]; then
        dd if=$BIN_DIR/image/bootlogo.fex of=$OTA_DIR/package_sys/bootlogo.img
        make_img_md5 $OTA_DIR/package_sys/bootlogo.img
    fi

    ota_tar="ota"$1".tar"
    echo "#####${ota_tar}#####"
    cd $OTA_DIR/package_sys/ && \
        tar -rvf ${ota_tar} boot.img && \
        tar -rvf ${ota_tar} boot.img.md5 && \
        tar -rvf ${ota_tar} rootfs.img && \
        tar -rvf ${ota_tar} rootfs.img.md5 && \
        tar -rvf ${ota_tar} recovery.img && \
        tar -rvf ${ota_tar} recovery.img.md5 && \
        tar -rvf ${ota_tar} boot0.img && \
        tar -rvf ${ota_tar} boot0.img.md5 && \
        tar -rvf ${ota_tar} uboot.img && \
        tar -rvf ${ota_tar} uboot.img.md5 && \
        if [ -f bootlogo.img ]; then
            tar -rvf ${ota_tar} bootlogo.img
            tar -rvf ${ota_tar} bootlogo.img.md5
        fi
        \cd $T
##############################################################

    if [ $need_usr -eq 1 ];then
        \cd $OTA_DIR && \
            tar -zcvf target_sys.tar.gz target_sys && \
            tar -zcvf ramdisk_sys.tar.gz ramdisk_sys && \
            tar -zcvf boot0_sys.tar.gz boot0_sys && \
            tar -zcvf uboot_sys.tar.gz uboot_sys && \
            tar -zcvf usr_sys.tar.gz usr_sys && \
            \cd $T
    else
        \cd $OTA_DIR && \
            tar -zcvf target_sys.tar.gz target_sys && \
            tar -zcvf ramdisk_sys.tar.gz ramdisk_sys && \
            tar -zcvf boot0_sys.tar.gz boot0_sys && \
            tar -zcvf uboot_sys.tar.gz uboot_sys && \
            \cd $T
    fi

    if [ $make_ota_fail -ne 0 ];then
        print_red "build ota package fail!"
    else
        print_green "build ota package finish!"
    fi
}

function make_ota_package_for_dual_app
{
	local T=$(gettop)
	local BIN_DIR=$T/out/${TARGET_BOARD}
	local OTA_DIR=$BIN_DIR/ota_dual_app
	local make_fail=0
	local ota_package=app_ota.tar
	mkdir -p "$OTA_DIR"
	\cd "$OTA_DIR"
	rm -f $ota_package $ota_package.md5
	target_list="$BIN_DIR/image/app.fex"
	for i in $target_list; do
		img=${i##*/}
		if [ ! -f "$i" ]; then
			print_red "$i not exsit!"
			make_fail=1
			break
		fi
		dd if="$i" of="$img" bs=512 conv=sync
		make_img_md5 "$img"
		tar -rvf "$ota_package" "$img"
		tar -rvf "$ota_package" "$img.md5"
	done
	if [ x$make_fail = x"1" ]; then
		print_red "make fail"
		rm -f $ota_package
	else
		make_img_md5 "$ota_package"
		print_red "$OTA_DIR/$ota_package"
	fi
	\cd - > /dev/null
}

function swupdate_init_key() {
	T=$(gettop)
	local password="swupdate"
	local SWUPDATE_CONFIG_DIR="$T/target/allwinner/${TARGET_BOARD}/swupdate"
	local BUSYBOX_BASEFILE_DIR="$T/target/allwinner/${TARGET_BOARD}/busybox-init-base-files"
	local PROCD_BASEFILE_DIR="$T/target/allwinner/${TARGET_BOARD}/base-files"
	mkdir -p $SWUPDATE_CONFIG_DIR
	\cd $SWUPDATE_CONFIG_DIR
	echo "-------------------- init password --------------------"
	if [ "$1" ] ; then
		password="$1"
	fi
	echo "$password" > swupdate_priv.password
	echo "-------------------- init priv key --------------------"
	openssl genrsa -aes256 -passout file:swupdate_priv.password -out swupdate_priv.pem
	echo "-------------------- init public key --------------------"
	openssl rsa -in swupdate_priv.pem -passin file:swupdate_priv.password -out swupdate_public.pem -outform PEM -pubout
	mkdir -p "$PROCD_BASEFILE_DIR/etc"
	cp swupdate_public.pem "$PROCD_BASEFILE_DIR/etc"
	mkdir -p "$BUSYBOX_BASEFILE_DIR/etc"
	cp swupdate_public.pem "$BUSYBOX_BASEFILE_DIR/etc"
	echo "-------------------- out files --------------------"
	echo "password:$(pwd)/swupdate_priv.password"
	echo "private key:$(pwd)/swupdate_priv.pem"
	echo "public key:$(pwd)/swupdate_public.pem"
	echo "public key:$PROCD_BASEFILE_DIR/swupdate_public.pem"
	echo "public key:$BUSYBOX_BASEFILE_DIR/swupdate_public.pem"

	\cd -
}

function make_recovery_img() {

	T=$(gettop)
	local recovery_img_config="$T/target/allwinner/${TARGET_BOARD}/defconfig_ota"

	#do some check
	if grep -q CONFIG_SUNXI_BOOT_IMAGE_NAME_SUFFIX_RECOVERY=y "$recovery_img_config"; then
		echo "have CONFIG_SUNXI_BOOT_IMAGE_NAME_SUFFIX_RECOVERY"
	else
		print_red "warning: no CONFIG_SUNXI_BOOT_IMAGE_NAME_SUFFIX_RECOVERY"
		echo -e "make ota_menuconfig"
		echo -e "\t--->  Target Images"
		echo -e "\t\t---> [*] customize image name"
		echo -e "\t\t\t---> Boot Image(kernel) name suffix (boot_recovery.img/boot_initramfs_recovery.img)"
	fi
	if grep -q CONFIG_TARGET_ROOTFS_INITRAMFS=y "$recovery_img_config"; then
		echo "have CONFIG_TARGET_ROOTFS_INITRAMFS"
	else
		print_red "warning: no CONFIG_TARGET_ROOTFS_INITRAMFS"
		echo -e "make ota_menuconfig"
		echo -e "\t---> Target Images"
		echo -e "\t\t---> [*] ramdisk"
		echo -e "\t\t\t---> Compression (xz)"
	fi

	#call make
	make V=s -j16 TARGET_CONFIG="$recovery_img_config"
	if [ $? -ne 0 ]; then
		print_red "make recovery img fail!"
	fi

}


function swupdate_make_recovery_img() {
	make_recovery_img "$@"
}

function swupdate_pack_swu() {

	T=$(gettop)
	local BIN_DIR=$T/out/${TARGET_BOARD}
	local SWU_DIR=$BIN_DIR/swupdate
	local SWUPDATE_CONFIG_DIR="$T/target/allwinner/${TARGET_BOARD}/swupdate"
	mkdir -p $SWUPDATE_CONFIG_DIR
	local TARGET_COMMON="$(awk -F "-" '{print $1}' <<< ${TARGET_BOARD})-common"
	local CFG="sw-subimgs$1.cfg"
	mkdir -p "$SWU_DIR"
	local storage_type_nor=0
	local f="$T/device/config/chips/${TARGET_PLATFORM}/configs/${TARGET_PLAN}/sys_config.fex"
	local B="$( awk -F"=" '/^storage_type/{print $2}' $f | sed 's/^[ \t]*//g' )"
	case $B in
		*0 | *5)
			local boot0_img=boot0_nand.fex
			;;
		*1 | *2 | *4)
			local boot0_img=boot0_sdcard.fex
			;;
		3)
			local boot0_img=boot0_spinor.fex
			storage_type_nor=1
			;;
		*)
			echo "###storage type error###"
			;;
	esac
	[ -n $boot0_img ] && {
		rm -rf $BIN_DIR/boot0.img
		dd if=$BIN_DIR/image/$boot0_img of=$BIN_DIR/boot0.img
	}
	local U="$(get_uboot)"
	if [[ "$U" =~ "2011" ]]; then
		local uboot_img=u-boot.fex
	else
		if [ x"$storage_type_nor" = x"1" ]; then
			local uboot_img=boot_package_nor.fex
		else
			local uboot_img=boot_package.fex
		fi
	fi
	rm -rf $BIN_DIR/uboot.img
	dd if=$BIN_DIR/image/$uboot_img of=$BIN_DIR/uboot.img

	if [ -e $SWUPDATE_CONFIG_DIR/$CFG ]; then
		local SWUPDATE_SUBIMGS="$SWUPDATE_CONFIG_DIR/$CFG"
	elif [ -e $T/target/allwinner/${TARGET_COMMON}/configs/$CFG ]; then
		local SWUPDATE_SUBIMGS="$T/target/allwinner/${TARGET_COMMON}/configs/$CFG"
	else
		local SWUPDATE_SUBIMGS="$T/target/allwinner/generic/configs/$CFG"
	fi

	echo "####$SWUPDATE_SUBIMGS####"
	. $SWUPDATE_SUBIMGS
	echo ${swota_file_list[@]} | sed 's/ /\n/g'

	[ ! -f "$SWUPDATE_SUBIMGS" ] && print_red "$SWUPDATE_SUBIMGS not exist!!" &&  return 1

	echo "-------------------- config --------------------"
	echo "subimgs config by: $SWUPDATE_SUBIMGS"
	echo "out dir: $SWU_DIR"

	echo "-------------------- do copy --------------------"
	cp "$SWUPDATE_SUBIMGS" "$SWU_DIR"
	rm -f "$SWU_DIR/sw-subimgs-fix.cfg"

	for line in ${swota_file_list[@]} ; do
		ori_file=$(echo $line | awk -F: '{print $1}')
		base_name=$(basename "$line")
		fix_name=${base_name#*:}
		cp $ori_file $SWU_DIR/$fix_name
		echo $fix_name >> "$SWU_DIR/sw-subimgs-fix.cfg"
	done

	\cd "$SWU_DIR"

	echo "-------------------- do sha256 --------------------"
	cp sw-description sw-description.bk
	while IFS= read -r line
	do
		item="$line"
		item_hash=$(sha256sum "$item" | awk '{print $1}')
		sed -i "s/\(.*\)\(sha256 = \"@$item\"\)/\1sha256 = \"$item_hash\"/g" sw-description
	done < "$SWU_DIR/sw-subimgs-fix.cfg"

	diff sw-description.bk sw-description

	echo "-------------------- do sign --------------------"

	local swupdate_need_sign=""
	grep "CONFIG_SWUPDATE_CONFIG_SIGNED_IMAGES=y" "$T/target/allwinner/${TARGET_BOARD}/defconfig" && {
		swupdate_need_sign=1
		echo "need do sign"
	}

	local swupdate_sign_method=""
	local password_para=""
	#for rsa
	local priv_key_file="$SWUPDATE_CONFIG_DIR/swupdate_priv.pem"
	local password_file="$SWUPDATE_CONFIG_DIR/swupdate_priv.password"
	#for cms
	local cert_cert_file="$SWUPDATE_CONFIG_DIR/swupdate_cert.cert.pem"
	local cert_key_file="$SWUPDATE_CONFIG_DIR/swupdate_cert.key.pem"

	[ x$swupdate_need_sign = x"1" ] && {
		echo "add sw-description.sig to sw-subimgs-fix.cfg"
		sed '1 asw-description.sig' -i sw-subimgs-fix.cfg
		grep "CONFIG_SWUPDATE_CONFIG_SIGALG_RAWRSA=y" "$T/target/allwinner/${TARGET_BOARD}/defconfig" && swupdate_sign_method="RSA"
		grep "CONFIG_SWUPDATE_CONFIG_SIGALG_CMS=y" "$T/target/allwinner/${TARGET_BOARD}/defconfig" && swupdate_sign_method="CMS"
		[ -e "$password_file" ] && {
			echo "password file exist"
			password_para="-passin file:$password_file"
		}

		if [ x"$swupdate_sign_method" = x"RSA" ]; then
			echo "generate sw-description.sig with rsa"
			openssl dgst -sha256 -sign "$priv_key_file" $password_para "$SWU_DIR/sw-description" > "$SWU_DIR/sw-description.sig"
		elif [ x"$swupdate_sign_method" = x"CMS" ]; then
			echo "generate sw-description.sig with cms"
			openssl cms -sign -in  "$SWU_DIR/sw-description" -out "$SWU_DIR/sw-description.sig" -signer "$cert_cert_file" \
				-inkey "$cert_key_file" -outform DER -nosmimecap -binary
		fi
	}


	echo "-------------------- do cpio --------------------"
	while IFS= read -r line
	do
		echo "$line"
	done < "$SWU_DIR/sw-subimgs-fix.cfg" | cpio -ov -H crc >  "$SWU_DIR/tina-${TARGET_BOARD}$1.swu"

	echo "-------------------- out file in --------------------"
	echo ""
	print_red "$SWU_DIR/tina-${TARGET_BOARD}$1.swu"
	du -sh "$SWU_DIR/tina-${TARGET_BOARD}$1.swu"
	echo ""

	\cd - > /dev/null
}

function make_swupdate_img()
{
	swupdate_pack_swu "$@"
}

function print_red(){
    echo -e '\033[0;31;1m'
    echo $1
    echo -e '\033[0m'
}

function print_green(){
    echo -e '\033[0;32;1m'
    echo $1
    echo -e '\033[0m'
}

function ota_general_keys()
{
    local target_dir=$1
    local key_name=OTA_Key.pem

    [ $# -lt 1 ] && echo "usage:ota_general_keys key_dir" && return 1
    rm -f "$target_dir"/*.pem
    #this is for test, finally we should manage ota key with other keys
    #general key
    openssl genrsa -out "$target_dir"/OTA_Key.pem -f4 2048
    #get public key
    openssl rsa -in "$target_dir"/OTA_Key.pem -pubout -out "$target_dir"/OTA_Key_pub.pem
    ls -ll "$target_dir"
    echo "done! please keep the key safe!"
}

function ota_sign_files()
{
    local target_dir=$1
    local key_dir=$2
    local key_name=OTA_Key.pem
    [ ! -n $3 ] && key_name=$3
    [ $# -lt 2 ] && echo "usage:ota_sign_files files_dir key_dir [key_name]" && return 1
    target_list="boot.img rootfs.img recovery.img boot_package.img boot0_nand.img boot0_sdcard.img"
    rm -f "$patch_dir"/*.signature
    for i in $target_list; do
        [ ! -f "$target_dir"/"$i" ] &&  print_red "$target_dir/$i is not exist!"
	echo "do signature for $target_dir/$i"
	openssl dgst -sha256 -out "$target_dir"/"$i.signature" -sign "$key_dir"/"$key_name" "$target_dir"/"$i"
    done
    ls -ll "$target_dir"
}

function ota_general_patchs()
{
    local old_file_dir="$1/ota/package_sys"
    local new_file_dir="$2/ota/package_sys"
    local patch_dir=$3
    local target_list="recovery.img boot.img rootfs.img "

    [ $# -lt 3 ] && echo "usage:ota_general_patchs old_file_dir new_file_dir patch_dir" && return 1

    rm -rf "$patch_dir"/*.patch "$patch_dir"/*.md5

    for i in $target_list; do
        [ ! -f "$old_file_dir"/"$i" ] && print_red "$old_file_dir/$i is not exist!"
	    [ ! -f "$new_file_dir"/"$i" ] && print_red "$new_file_dir/$i is not exist!"
	    echo "Generating patch for $i"
	    bsdiff "$old_file_dir"/"$i" "$new_file_dir"/"$i" "$patch_dir"/"$i.patch"
	    cp "$new_file_dir"/"$i.md5" "$patch_dir"/"$i.md5"
    done

    ls -ll "$patch_dir"

    print_green "Generate patch finished!!!"
}

function make_ramfs() {
    local T=$(gettop)
    local make_ramfs_fail=0

    local ramfs_img_config="$T/target/allwinner/${TARGET_BOARD}/defconfig_ramfs"

    print_red "build ramfs img"

    #call make
    make V=s -j16 TARGET_CONFIG="$ramfs_img_config"
    if [ $? -ne 0 ]; then
	print_red "make ramfs img fail!"
	make_ramfs_fail=1
    fi

    if [ $make_ramfs_fail -ne 0 ];then
        print_red "build ramfs fail!"
    else
        print_red "build ramfs finish!"
        print_red "cp  $T/out/${TARGET_BOARD}/compile_dir/target/rootfs to $T/out/${TARGET_BOARD}/compile_dir/target/rootfs_ramfs"
        rm -rf "$T/out/${TARGET_BOARD}/compile_dir/target/rootfs_ramfs"
        cp -fpr "$T/out/${TARGET_BOARD}/compile_dir/target/rootfs"  "$T/out/${TARGET_BOARD}/compile_dir/target/rootfs_ramfs"
        du -sh "$T/out/${TARGET_BOARD}/compile_dir/target/rootfs_ramfs"
    fi

    \cd "$T/out/${TARGET_BOARD}/compile_dir/target/"
    ramfs_cpio=rootfs_ramfs.cpio.none
    #ramfs_cpio=rootfs_ramfs.cpio.gz
    #ramfs_cpio=rootfs_ramfs.cpio.xz
    rm -f ${ramfs_cpio}
    ln -s rootfs_ramfs skel
    ../../../../scripts/build_rootfs.sh c ${ramfs_cpio}
    mv ${ramfs_cpio} "$T/target/allwinner/${TARGET_BOARD}/"
    \cd "$T"
    du -sh "$T/target/allwinner/${TARGET_BOARD}/${ramfs_cpio}"
}

pack_usage()
{
	printf "Usage: pack [-cCHIP] [-pPLATFORM] [-bBOARD] [-d] [-s] [-m] [-w] [-i] [-h]
	-c CHIP (default: $chip)
	-p PLATFORM (default: $platform)
	-b BOARD (default: $board)
	-d pack firmware with debug info output to card0
	-s pack firmware with signature
	-m pack dump firmware
	-w pack programmer firmware
	-i pack sys_partition.fex downloadfile img.tar.gz
	-h print this help message
"
}

function pack() {
	local T=$(gettop)
	local chip=sun5i
	local platform=$(get_build_var TARGET_BUILD_VARIANT)
	local board_platform=$(get_build_var TARGET_BOARD_PLATFORM)
	local board=$(get_build_var TARGET_BOARD)
	local debug=uart0
	local sigmode=none
	local securemode=none
	local mode=normal
	local programmer=none
	local tar_image=none
	unset OPTIND
	while getopts "dsvmwih" arg
	do
		case $arg in
			d)
				debug=card0
				;;
			s)
				sigmode=secure
				;;
			v)
				securemode=secure
				;;
			m)
				mode=dump
				;;
			w)
				programmer=programmer
				;;
			i)
				tar_image=tar_image
				;;
			h)
				pack_usage
				return 0
				;;
			?)
			return 1
			;;
		esac
	done

	chip=${TARGET_CHIP}
	if [ "x$chip" = "x" ]; then
		echo "platform($TARGET_PLATFORM) not support"
		return
	fi

	$T/scripts/pack_img.sh -c $chip -p $platform -b $board \
		-d $debug -s $sigmode -m $mode -w $programmer -v $securemode -i $tar_image -t $T
}

function createkeys()
{
	local T=$(gettop)
	local board=$(get_build_var TARGET_BOARD)
	$T/scripts/createkeys -b $board -t $T
}

function croot()
{
    T=$(gettop)
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    \cd $T
}

function cboot()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    [ -z "${TARGET_UBOOT}" ] && "Please lunch your combo firstly" && return

    if [ "${TARGET_UBOOT}" = "u-boot-2018" ]; then
        \cd $(gettop)/lichee/brandy-2.0/${TARGET_UBOOT}
    else
        \cd $(gettop)/lichee/brandy/${TARGET_UBOOT}
    fi
}

function cboot0()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    [ -z "${TARGET_UBOOT}" ] && "Please lunch your combo firstly" && return

    if [ "${TARGET_UBOOT}" = "u-boot-2014.07" ]; then
        \cd $(gettop)/lichee/bootloader/uboot_2014_sunxi_spl/
    elif [ "${TARGET_UBOOT}" = "u-boot-2011.09" ]; then
        \cd $(gettop)/lichee/bootloader/uboot_2011_sunxi_spl/
    elif [ "${TARGET_UBOOT}" = "u-boot-2018" ]; then
        \cd $T/lichee/brandy-2.0/spl/
    fi
}

function cbin()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    local tina_bin_dir="$T/target/allwinner/${TARGET_BOARD%-*}-common/bin"
    local longan_bin_dir="$T/device/config/chips/${TARGET_PLATFORM}/bin"

    [ -e $longan_bin_dir -a -e $tina_bin_dir ] && {
        print_red "warning: both longan and tina bin dir exist"
        print_red "tina: $tina_bin_dir"
        print_red "longan: $longan_bin_dir"
    }

    [ -e $tina_bin_dir ] && \cd $tina_bin_dir && return
    [ -e $longan_bin_dir ] && \cd $longan_bin_dir && return
}

function cdts()
{
    local T=$(gettop)
    local S=
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    local K="linux-${TARGET_KERNEL_VERSION}"
    local A="${TINA_TARGET_ARCH}"
    [ -z "$K" -o -z "$A" ] && "Please lunch your combo firstly" && return
    [ "$A" = "aarch64" ] && {
        S=sunxi
        A=arm64
    }

    \cd $T/lichee/$K/arch/$A/boot/dts/$S
}

function ckernel
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $T/lichee/linux-${TARGET_KERNEL_VERSION}/
}

function cgeneric
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    \cd $T/target/allwinner/generic
}

function callwinnerpk
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    [ -z "${TARGET_BOARD}" ] && "Please lunch your combo firstly" && return

    \cd $T/package/allwinner
}

function ctinatest
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    [ -z "${TARGET_BOARD}" ] && "Please lunch your combo firstly" && return

    \cd $T/package/testtools/tinatest
}

function cdevice
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    [ -z "${TARGET_BOARD}" ] && "Please lunch your combo firstly" && return

    \cd $T/target/allwinner/${TARGET_BOARD}
}

function ccommon
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    [ -z "${TARGET_BOARD}" ] && "Please lunch your combo firstly" && return

    \cd $T/target/allwinner/${TARGET_PLATFORM}-common
}

function cconfigs
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    [ -z "${TARGET_BOARD}" ] && "Please lunch your combo firstly" && return

    local tina_config_dir="$T/target/allwinner/${TARGET_BOARD}/configs"
    local longan_config_dir="$T/device/config/chips/${TARGET_PLATFORM}/configs/${TARGET_BOARD#*-}/linux"

    [ -e $longan_config_dir -a -e $tina_config_dir ] && {
        print_red "warning: both longan and tina configs dir exist"
        print_red "tina: $tina_config_dir"
        print_red "longan: $longan_config_dir"
    }

    [ -e $tina_config_dir ] && \cd $tina_config_dir && return
    [ -e $longan_config_dir ] && \cd $longan_config_dir && return
}

function ctoolchain()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    [ -z "${TARGET_BOARD}" ] && "Please lunch your combo firstly" && return

    local A=${TINA_TARGET_ARCH}
    local C="$(awk -F\" '/CONFIG_LIBC=/{print $2}' \
        $T/target/allwinner/${TARGET_BOARD}/defconfig)"

    if [ "$TARGET_PLATFORM" = "r6" -o "$TARGET_PLATFORM" = "c200s" -o "$TARGET_PLATFORM" = "v133" -o "$TARGET_PLATFORM" = "c600" ]; then
	    C=arm9-$C
    fi
    \cd $T/prebuilt/gcc/linux-x86/$A/toolchain-sunxi-$C/toolchain
}

function crootfs()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    [ -z "${TARGET_BOARD}" ] && "Please lunch your combo firstly" && return

    \cd $T/out/${TARGET_BOARD}/compile_dir/target/rootfs
}

function cout()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    [ -z "${TARGET_BOARD}" ] && "Please lunch your combo firstly" && return

    \cd $T/out/${TARGET_BOARD}
}

function cgrep()
{
    find . -name .repo -prune -o -name .git -prune -o -name out -prune -o -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.h' \) -print0 | xargs -0 grep --color -n "$@"
}

function ctarget()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    [ -z "${TARGET_BOARD}" ] && "Please lunch your combo firstly" && return

    \cd $T/out/${TARGET_BOARD}/compile_dir/target
}

function add-rootfs-demo()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return
    cp -rf $(gettop)/package/add-rootfs-demo/*  $(gettop)/out/${TARGET_BOARD}/compile_dir/target/rootfs
    rm $(gettop)/out/${TARGET_BOARD}/compile_dir/target/rootfs/README
    $(gettop)/out/host/bin/mksquashfs4          $(gettop)/out/${TARGET_BOARD}/compile_dir/target/rootfs \
                    $(gettop)/out/${TARGET_BOARD}/root.squashfs -noappend -root-owned -comp xz -b 256k \
                                        -p '/dev d 755 0 0' -p '/dev/console c 600 0 0 5 1' -processors 1
    rm  $(gettop)/out/${TARGET_BOARD}/rootfs.img
    dd if=$(gettop)/out/${TARGET_BOARD}/root.squashfs of=$(gettop)/out/${TARGET_BOARD}/rootfs.img bs=128k conv=sync
}

function mkrootfs_squashfs4()
{
	kernelfs_formate=`grep CONFIG_SQUASHFS=y $(gettop)/lichee/*/.config | cut -d ":" -f 2`
	echo -e "\033[31m$kernelfs_formate\033[0m"
	if [ -z $kernelfs_formate ];then
		echo -e "\033[31m run -make kernel_menuconfig- choice "squashfs" first!\033[0m"
	else
		compression=`grep ^CONFIG_KERNEL.*y$ $(gettop)/.config | awk 'NR==1{print}' | sed -r 's/.*_(.*)=.*/\1/' | tr '[A-Z]' '[a-z]'`
		if [ -n "$compression" ];then
			$(gettop)/out/host/bin/mksquashfs4  $(gettop)/out/${TARGET_BOARD}/compile_dir/target/rootfs  $(gettop)/out/${TARGET_BOARD}/root.squashfs \
									-noappend -root-owned -comp $compression -b 256k -p '/dev d 755 0 0' -p '/dev/console c 600 0 0 5 1' -processors 1
		else
			$(gettop)/out/host/bin/mksquashfs4  $(gettop)/out/${TARGET_BOARD}/compile_dir/target/rootfs $(gettop)/out/${TARGET_BOARD}/root.squashfs  \
											-noappend -root-owned -comp xz -b 256k -p '/dev d 755 0 0' -p '/dev/console c 600 0 0 5 1' -processors 1
		fi
		rm  $(gettop)/out/${TARGET_BOARD}/rootfs.img
		dd if=$(gettop)/out/${TARGET_BOARD}/root.squashfs of=$(gettop)/out/${TARGET_BOARD}/rootfs.img bs=128k conv=sync
	fi
}

function mkrootfs_ext4()
{
    kernelfs_formate=`grep CONFIG_EXT4_FS=y $(gettop)/lichee/*/.config | cut -d ":" -f 2`
    echo -e "\033[32m$kernelfs_formate\033[0m"
    if [ -z $kernelfs_formate ];then
		echo -e "\033[31m run -make kernel_menuconfig- choice "ext4fs" first!\033[0m"
	else
        if [ -f $(gettop)/.config ];then
            local rootfs_size_m=`awk -F'=' '/CONFIG_TARGET_ROOTFS_PARTSIZE/{print $2}' $(gettop)/.config`
        else
            echo -e "run make menuconfig first!" && return
        fi
        dd if=/dev/zero of=$(gettop)/out/${TARGET_BOARD}/root.ext4 count=$rootfs_size_m bs=1M
        $(gettop)/out/host/bin/mkfs.ext4 -b 4096 $(gettop)/out/${TARGET_BOARD}/root.ext4 -d $(gettop)/out/${TARGET_BOARD}/compile_dir/target/rootfs
        $(gettop)/out/host/bin/fsck.ext4 -pvfD $(gettop)/out/${TARGET_BOARD}/root.ext4
        rm  $(gettop)/out/${TARGET_BOARD}/rootfs.img
        dd if=$(gettop)/out/${TARGET_BOARD}/root.ext4 of=$(gettop)/out/${TARGET_BOARD}/rootfs.img bs=128k conv=sync
    fi
}

function add-prebuilts-to-rootfs()
{
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    if [ $# -eq 0 ];then
        echo -e "\033[31mPlease run add-prebuilts-to-rootfs [nor|ext4]\033[0m"
        return
    fi

	[ -d $(gettop)/target/allwinner/${TARGET_BOARD}/prebuilts ] && {
        cp -rf $(gettop)/target/allwinner/${TARGET_BOARD}/prebuilts/*  $(gettop)/out/${TARGET_BOARD}/compile_dir/target/rootfs
        rm $(gettop)/out/${TARGET_BOARD}/compile_dir/target/rootfs/README
        echo -e "\033[32mCopying prebuilt files to target ...\033[0m"
    }

    if [ x"$1" = x"nor" ];then
        mkrootfs_squashfs4
    fi

    if [ x"$1" = x"ext4" ];then
        mkrootfs_ext4
    fi
}

function mkrootfs_jffs2()
{
	kernelfs_formate=`grep CONFIG_JFFS2_FS=y $(gettop)/lichee/*/.config | cut -d ":" -f 2`
	echo -e "\033[31m$kernelfs_formate\033[0m"
	if [ -z $kernelfs_formate ];then
		echo -e "\033[31m run -make kernel_menuconfig- choice "jffs2fs" first!\033[0m"
	else
		$(gettop)/out/host/bin/mkfs.jffs2  --little-endian --squash-uids -v -X rtime -x zlib -x lzma -D $(gettop)/build/device_table.txt \
					-e 128KiB -o $(gettop)/out/${TARGET_BOARD}/root.jffs2-128k -d $(gettop)/out/${TARGET_BOARD}/compile_dir/target/rootfs \
																										-v 2>&1 1>/dev/null | awk '/^.+$/'
		rm  $(gettop)/out/${TARGET_BOARD}/rootfs.img
		dd if=$(gettop)/out/${TARGET_BOARD}/root.jffs2-128k of=$(gettop)/out/${TARGET_BOARD}/rootfs.img bs=128k conv=sync
	fi
}

function recomp_rootfs()
{
    T=$(gettop)
    local T=$(gettop)
    [ -z "$T" ] \
        && echo "Couldn't locate the top of the tree.  Try setting TOP." \
        && return

    file_formate=`grep ^CONFIG_TARGET_ROOTFS.*y $(gettop)/.config | cut -d "_" -f 4 | grep "=" | sed -r 's/(.*)=.*/\1/'`
    echo -e "\033[31m $file_formate\033[0m"
    if [ -z $file_formate ];then
        echo -e "\033[31m run -make menuconfig- choice fs_formate of target images!\033[0m"
    else
        [ x$file_formate = x"SQUASHFS" ] && mkrootfs_squashfs4
        [ x$file_formate = x"EXT4FS" ] && mkrootfs_ext4
        [ x$file_formate = x"JFFS2" ] && mkrootfs_jffs2
    fi
}

function godir ()
{
    if [[ -z "$1" ]]; then
        echo "Usage: godir <regex>"
        return
    fi
    local T=$(gettop)
    if [[ ! -f $T/filelist ]]; then
        echo -n "Creating index..."
        (\cd $T; find . -wholename ./out -prune -o -wholename ./.repo -prune -o -type f > filelist)
        echo " Done"
        echo ""
    fi
    local lines
    lines=($(\grep "$1" $T/filelist | sed -e 's/\/[^/]*$//' | sort | uniq))
    if [[ ${#lines[@]} = 0 ]]; then
        echo "Not found"
        return
    fi
    local pathname
    local choice
    if [[ ${#lines[@]} > 1 ]]; then
        while [[ -z "$pathname" ]]; do
            local index=1
            local line
            for line in ${lines[@]}; do
                printf "%6s %s\n" "[$index]" $line
                index=$(($index + 1))
            done
            echo
            echo -n "Select one: "
            unset choice
            read choice
            if [[ $choice -gt ${#lines[@]} || $choice -lt 1 ]]; then
                echo "Invalid choice"
                continue
        fi
            pathname=${lines[$(($choice-1))]}
        done
    else
        pathname=${lines[0]}
    fi
    \cd $T/$pathname
}

function make()
{
    local start_time=$(date +"%s")
    command make V=s "$@"
    local ret=$?
    local end_time=$(date +"%s")
    local tdiff=$(($end_time-$start_time))
    local hours=$(($tdiff / 3600 ))
    local mins=$((($tdiff % 3600) / 60))
    local secs=$(($tdiff % 60))
    echo
    if [ $ret -eq 0 ] ; then
        echo -n -e "#### make completed successfully "
    else
        echo -n -e "#### make failed to build some targets "
    fi
    if [ $hours -gt 0 ] ; then
        printf "(%02g:%02g:%02g (hh:mm:ss))" $hours $mins $secs
    elif [ $mins -gt 0 ] ; then
        printf "(%02g:%02g (mm:ss))" $mins $secs
    elif [ $secs -gt 0 ] ; then
        printf "(%s seconds)" $secs
    fi
    echo -e " ####"
    echo
    return $ret
}

[ -e ./build/.hooks/expand_func ] &&
    source ./build/.hooks/expand_func

#### MAIN ####
envsetup
