#! /bin/sh

AIOS_HOME="."
export LUA_PATH=""
export LUA_CPATH="luaclib/awr16/?.so;"
#export LD_LIBRARY_PATH=/tmp/aispeech/test/tinaplayer/lib


if [[ $1 == 'start' ]]; then

    #ps | awk '!/awk/ && /.\/bin\/lasa .*.lua/ {print $1}' | while read pid; do kill -9 $pid ; done
    ./bin/lasa test/exit.lua
    sleep 1
    killall lasa

    find $AIOS_HOME/init -type f | awk '!/\.svn/ && /\.lua$/' | sort | while read s; do
        echo "$s - starting"
        ./bin/lasa $s 2>&1 & #| tee .bootloader.`basename $s`.log &
        if [[ $s == './init/00-bus-server.lua' ]]; then
            echo "sleeping 1 seconds"
            sleep 1
        fi
        #sleep 1
    done
    sleep 1

#    /tmp/aispeech/test/tinaplayer/tinaplayer &

elif [[ $1 == 'stop' ]]; then
    ./bin/lasa test/exit.lua
    sleep 1
    killall lasa
else
    echo "usage: $0 start|stop"
fi

