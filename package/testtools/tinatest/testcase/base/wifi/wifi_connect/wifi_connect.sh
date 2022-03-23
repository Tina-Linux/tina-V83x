#!/bin/sh

wifi_connect()
{
    echo "====== start wifi_connect_test ======"
    wifi_connect_ap_test AWTest 1qaz@WSX
    if [ $? -eq 0 ];then
	echo "====== wifi_connect successed!!!======"
	exit 0
    else
	echo "====== wifi_connect failed!!! ======"
	exit 1
    fi
}

wifi_connect
