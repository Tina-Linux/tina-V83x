#!/bin/sh

wifi_remove_all_network()
{
    echo "====== start wifi_remove_all_networks_test ======"
    wifi_remove_all_networks_test
    if [ $? -eq 0 ];then
	echo "====== wifi_remove_all_networks_test successed!!! ======"
	exit 0
    else
	echo "====== wifi_remove_all_networks_test failed!!! ======"
	exit 1
    fi
}

wifi_remove_all_network
