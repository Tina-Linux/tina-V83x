#!/bin/bash

#GVFB=gvfb

export NCS_CFG_PATH=$PREFIX/etc

# use ./MiniGUI.cfg
if [ ${MG_CFG_FILE}x != 'x' -o ${MG_CFG_PATH}x != 'x' ]; then
    echo "Warning: Going to unset MG_CFG_FILE and MG_CFG_PATH to make sure ./MiniGUI.cfg is used."
    echo 
    unset MG_CFG_FILE
    unset MG_CFG_PATH
fi

# gvfb
#which $GVFB > /dev/null 2>&1
#if [ $? -ne 0 ]; then
#    echo "Fatal: please put '$GVFB' into \$PATH"
#    exit 1
#fi

# MG_RES_PATH
if [ ${MG_RES_PATH}x == 'x' -a ! -e /usr/res/ ]; then
    echo "Fatal: please set \$MG_RES_PATH to the path where you put MiniGUI resource files."
    echo "       e.g. \"export MG_RES_PATH=/home/target/share/minigui/res/\""
    echo 
    exit 1
fi

# go go go !!!
echo "Now we are going to launch CellphoneUXDemo in *FULLSCREEN* mode."
echo "You can get back to desktop by pressing <F11>"
echo 
#echo ">>> Press <ENTER> to continue ..."
#read

#./cellphone > /tmp/cell.log 2>&1
./cellphone
