#!/bin/bash
function print_red(){
    echo -e '\033[0;31;1m'
    echo $1
    echo -e '\033[0m'
}
function mm() {
    local T=$1/
    local orgin=`pwd`
    local path=`pwd`
    #find target makefile
    #trap 'echo $orgin >> ~/mm; trap - SIGINT; ' SIGINT
    while [ x`pwd` != x$T ] && [ x`pwd` != x"/" ]
    do
        find  -maxdepth 1 -name Makefile | xargs cat | grep "define Package" > /dev/null
        is_package=$?
        find  -maxdepth 1 -name Makefile | xargs cat | grep "define KernelPackage" > /dev/null
        is_kernel_package=$?
        if [ $is_package -eq 1 ] && [ $is_kernel_package -eq 1 ]; then
            cd ../
        else
            path=`pwd`
            target=${path#*$T}
            cd $T
            cmd="install V=s"
            for i in $*; do
                [ x$i = x"-B" ] && {
                    # -B clean the package
                    print_red "make $target/clean V=s"
                    make $target/clean V=s
                }
                [ x${i:0:2} = x"-j" ] && cmd=$cmd" "$i
            done
            print_red "make $target/$cmd"
            make $target/$cmd
            cd $orgin
            #trap - SIGINT
            return
        fi
    done
    cd $orgin
    #trap - SIGINT
    print_red "Can't not find Tina Package Makefile!"
}
mm $*
