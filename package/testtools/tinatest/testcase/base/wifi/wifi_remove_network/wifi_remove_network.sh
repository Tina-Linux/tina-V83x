#!/bin/sh

wifi_remove_network()
{
    echo "====== start wifi_remove_network_test ======"
    wifi_remove_network_test AWTest 2
    if [ $? -eq 0 ];then
	echo "====== wifi_remove_network_test successed!!! ======"
	exit 0
    else
	echo "====== wifi_remove_network_test failed!!! ======"
	exit 1
    fi
}

wifi_remove_network
